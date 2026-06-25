#include "xnode_auto_save.h"
#include "xnode_subtree_subscription.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <optional>

namespace xsdk::impl {

namespace {

    class NodeAutoSaveImpl final: public xnode::INodeAutoSave, public std::enable_shared_from_this<NodeAutoSaveImpl> {
    public:
        explicit NodeAutoSaveImpl(xnode::NodeAutoSaveParams&& _params)
            : node_(std::move(_params.node)),
              save_(std::move(_params.save)),
              debounce_(std::max<xbase::Time64>(_params.debounce, 0)),
              max_interval_(std::max<xbase::Time64>(_params.max_interval, 0)),
              scheduler_p_(xscheduler::DefaultScheduler(_params.scheduler_p)),
              worker_(std::move(_params.worker))
        {
        }

        ~NodeAutoSaveImpl() override { Stop(true); }

        bool Start()
        {
            if (!node_ || !save_ || !scheduler_p_)
                return false;

            const auto weak_this = weak_from_this();
            auto       on_change = [weak_this](const INode::CallbackReason _reason,
                                               const INode::SPtrC&,
                                               const XKey&,
                                               const XValueRT&,
                                               const XValueRT&) {
                const auto self = weak_this.lock();
                if (self)
                    self->OnSubtreeChange_(_reason);
            };
            subscription_ = xnode::CreateNodeSubtreeSubscription({node_, std::move(on_change), true});

            return subscription_ && subscription_->Active();
        }

        void Flush() override
        {
            xscheduler::StopTask(task_uid_, scheduler_p_, true);

            bool save_now = false;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_)
                    return;

                save_now     = dirty_;
                dirty_       = false;
                first_dirty_ = time64::kNoVal;
                last_change_ = time64::kNoVal;
                rollback_state_.reset();
            }

            if (save_now)
                SaveNow_();
            else
                WaitForSave_();
        }

        void Stop(const bool _flush_pending) override
        {
            bool flush_now = false;
            bool wait_only = false;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_) {
                    wait_only = _flush_pending;
                }
                else {
                    stopped_  = true;
                    flush_now = _flush_pending && dirty_;
                }
            }

            if (wait_only) {
                WaitForSave_();
                return;
            }

            if (subscription_)
                subscription_->Stop();

            xscheduler::StopTask(task_uid_, scheduler_p_, true);

            if (flush_now)
                SaveNow_();
            else if (_flush_pending)
                WaitForSave_();
        }

        bool Active() const override
        {
            const std::unique_lock lck(mutex_);
            return !stopped_;
        }

        std::error_code LastError() const override
        {
            const std::unique_lock lck(mutex_);
            return last_error_;
        }

    private:
        struct DirtyState {
            bool          dirty       = false;
            xbase::Time64 first_dirty = time64::kNoVal;
            xbase::Time64 last_change = time64::kNoVal;
        };

        void OnSubtreeChange_(const INode::CallbackReason _reason)
        {
            if (_reason == INode::CallbackReason::Rollback) {
                RollbackDirty_();
                return;
            }

            MarkDirty_(_reason == INode::CallbackReason::Changes);
        }

        void MarkDirty_(const bool _rollbackable)
        {
            xbase::Time64 target_time = time64::kNoVal;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_)
                    return;

                if (_rollbackable)
                    rollback_state_ = DirtyState {dirty_, first_dirty_, last_change_};
                else
                    rollback_state_.reset();

                const auto now = scheduler_p_->Clock()->Time();
                if (!dirty_)
                    first_dirty_ = now;

                dirty_       = true;
                last_change_ = now;
                target_time  = TargetTime_();
            }

            Schedule_(target_time);
        }

        void RollbackDirty_()
        {
            bool          stop_task   = false;
            xbase::Time64 target_time = time64::kNoVal;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_ || !rollback_state_)
                    return;

                dirty_       = rollback_state_->dirty;
                first_dirty_ = rollback_state_->first_dirty;
                last_change_ = rollback_state_->last_change;
                rollback_state_.reset();

                if (dirty_)
                    target_time = TargetTime_();
                else
                    stop_task = true;
            }

            if (stop_task)
                xscheduler::StopTask(task_uid_, scheduler_p_, false);
            else
                Schedule_(target_time);
        }

        void Schedule_(const xbase::Time64 _target_time)
        {
            if (!time64::IsValid(_target_time))
                return;

            {
                const std::unique_lock state_lck(mutex_);
                if (stopped_)
                    return;
            }

            const auto weak_this = weak_from_this();
            xscheduler::RunTaskNoLaterThan(
                task_uid_,
                scheduler_p_,
                _target_time,
                [weak_this](const xbase::IScheduler::TaskInfo*) -> std::optional<xbase::Time64> {
                    const auto self = weak_this.lock();
                    return self ? self->OnScheduledTask_() : std::nullopt;
                },
                {},
                worker_);
        }

        std::optional<xbase::Time64> OnScheduledTask_()
        {
            bool save_now = false;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_ || !dirty_) {
                    task_uid_.store(xbase::kInvalidUid);
                    return std::nullopt;
                }

                const auto now         = scheduler_p_->Clock()->Time();
                const auto target_time = TargetTime_();
                if (time64::IsValid(target_time) && now < target_time)
                    return target_time;

                dirty_       = false;
                first_dirty_ = time64::kNoVal;
                last_change_ = time64::kNoVal;
                rollback_state_.reset();
                task_uid_.store(xbase::kInvalidUid);
                save_now = true;
            }

            if (save_now)
                SaveNow_();

            return std::nullopt;
        }

        xbase::Time64 TargetTime_() const
        {
            if (!dirty_ || !time64::IsValid(last_change_))
                return time64::kNoVal;

            auto target_time = last_change_ + debounce_;
            if (max_interval_ > 0 && time64::IsValid(first_dirty_))
                target_time = std::min(target_time, first_dirty_ + max_interval_);

            return target_time;
        }

        void SaveNow_()
        {
            const auto node = Node_();
            if (!node)
                return;

            const std::unique_lock save_lck(save_mutex_);
            const auto             error = save_(node);
            {
                const std::unique_lock lck(mutex_);
                last_error_ = error;
            }
        }

        void WaitForSave_() { const std::unique_lock save_lck(save_mutex_); }

        INode::SPtrC Node_() const
        {
            const std::unique_lock lck(mutex_);
            return node_;
        }

        const INode::SPtr                     node_;
        const xnode::INodeAutoSave::SavePF    save_;
        const xbase::Time64                   debounce_;
        const xbase::Time64                   max_interval_;
        xbase::IScheduler* const              scheduler_p_;
        const xbase::IWorker::SPtr            worker_;
        xnode::INodeSubtreeSubscription::SPtr subscription_;
        mutable std::mutex                    mutex_;
        std::mutex                            save_mutex_;
        std::atomic<xbase::Uid>               task_uid_ {xbase::kInvalidUid};
        bool                                  stopped_     = false;
        bool                                  dirty_       = false;
        xbase::Time64                         first_dirty_ = time64::kNoVal;
        xbase::Time64                         last_change_ = time64::kNoVal;
        std::optional<DirtyState>             rollback_state_;
        std::error_code                       last_error_;
    };

} // namespace

} // namespace xsdk::impl

namespace xsdk::xnode {

INodeAutoSave::SPtr CreateNodeAutoSave(NodeAutoSaveParams&& _params)
{
    auto auto_save = std::make_shared<impl::NodeAutoSaveImpl>(std::move(_params));
    return auto_save->Start() ? auto_save : nullptr;
}

} // namespace xsdk::xnode
