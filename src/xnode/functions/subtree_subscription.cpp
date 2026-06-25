#include "xnode_functions.h"
#include "xnode_subtree_subscription.h"

#include <map>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace xsdk::impl {

namespace {

    class NodeSubtreeSubscriptionImpl final: public xnode::INodeSubtreeSubscription,
                                             public std::enable_shared_from_this<NodeSubtreeSubscriptionImpl> {
    public:
        explicit NodeSubtreeSubscriptionImpl(xnode::NodeSubtreeSubscriptionParams&& _params)
            : root_(std::move(_params.node)),
              on_change_(std::move(_params.on_change)),
              subscribe_existing_nodes_(_params.subscribe_existing_nodes)
        {
        }

        ~NodeSubtreeSubscriptionImpl() override { Stop(); }

        bool Start()
        {
            if (!root_ || !on_change_)
                return false;

            SubscribeNode_(root_, subscribe_existing_nodes_);
            return Active();
        }

        void Stop() override
        {
            std::vector<std::pair<INode::SPtr, uint64_t>> watches;
            {
                const std::unique_lock lck(mutex_);
                if (stopped_)
                    return;

                stopped_ = true;
                for (auto& [uid, watch] : watches_)
                    watches.emplace_back(std::move(watch.node), watch.callback_uid);
                watches_.clear();
            }

            for (auto& [node, callback_uid] : watches) {
                if (node && callback_uid)
                    node->OnChangeRemove(callback_uid);
            }
        }

        bool Active() const override
        {
            const std::unique_lock lck(mutex_);
            return !stopped_ && !watches_.empty();
        }

    private:
        struct Watch {
            INode::SPtr node;
            uint64_t    callback_uid = 0;
        };

        void RemoveCallback_(const Watch& _watch) const
        {
            if (_watch.node && _watch.callback_uid)
                _watch.node->OnChangeRemove(_watch.callback_uid);
        }

        std::optional<Watch> TakeWatch_(const INode::SPtr& _node)
        {
            const std::unique_lock lck(mutex_);
            const auto             it = watches_.find(_node->ObjectUid());
            if (it == watches_.end())
                return std::nullopt;

            auto watch = std::move(it->second);
            watches_.erase(it);
            return watch;
        }

        void SubscribeNode_(const INode::SPtr& _node, const bool _include_children)
        {
            if (!_node)
                return;

            const auto node_uid = _node->ObjectUid();
            {
                const std::unique_lock lck(mutex_);
                if (stopped_ || watches_.find(node_uid) != watches_.end())
                    return;

                watches_[node_uid] = Watch {_node, 0};
            }

            const auto weak_this    = weak_from_this();
            const auto callback_uid = _node->OnChangeAdd([weak_this](const INode::CallbackReason _reason,
                                                                     const INode::SPtrC&         _node,
                                                                     const XKey&                 _key,
                                                                     const XValueRT&             _prev_value,
                                                                     const XValueRT&             _new_value) {
                const auto self = weak_this.lock();
                if (!self)
                    return std::optional<bool>();

                return self->OnNodeChange_(_reason, _node, _key, _prev_value, _new_value);
            });

            if (!callback_uid) {
                TakeWatch_(_node);
                return;
            }

            bool remove_callback = false;
            {
                const std::unique_lock lck(mutex_);
                const auto             it = watches_.find(node_uid);
                if (stopped_ || it == watches_.end()) {
                    remove_callback = true;
                }
                else {
                    it->second.callback_uid = callback_uid;
                }
            }

            if (remove_callback) {
                _node->OnChangeRemove(callback_uid);
                return;
            }

            if (!_include_children)
                return;

            for (const auto& child : xnode::ChildNodesGet(_node))
                SubscribeNode_(child, true);
        }

        void UnsubscribeNode_(const INode::SPtr& _node)
        {
            if (!_node)
                return;

            if (auto watch = TakeWatch_(_node))
                RemoveCallback_(*watch);

            for (const auto& child : xnode::ChildNodesGet(_node))
                UnsubscribeNode_(child);
        }

        std::optional<bool> OnNodeChange_(const INode::CallbackReason _reason,
                                          const INode::SPtrC&         _node,
                                          const XKey&                 _key,
                                          const XValueRT&             _prev_value,
                                          const XValueRT&             _new_value)
        {
            bool stopped = false;
            {
                const std::unique_lock lck(mutex_);
                stopped = stopped_;
            }

            if (stopped)
                return std::optional<bool>();

            {
                const auto prev_node = _prev_value.QueryPtr<INode>();
                const auto new_node  = _new_value.QueryPtr<INode>();

                if (prev_node && (!new_node || prev_node->ObjectUid() != new_node->ObjectUid()))
                    UnsubscribeNode_(prev_node);
                if (new_node)
                    SubscribeNode_(new_node, true);
            }

            on_change_(_reason, _node, _key, _prev_value, _new_value);

            return true;
        }

        const INode::SPtr                  root_;
        const xnode::OnNodeSubtreeChangePF on_change_;
        const bool                         subscribe_existing_nodes_ = true;
        mutable std::mutex                 mutex_;
        bool                               stopped_ = false;
        std::map<uint64_t, Watch>          watches_;
    };

} // namespace

} // namespace xsdk::impl

namespace xsdk::xnode {

INodeSubtreeSubscription::SPtr CreateNodeSubtreeSubscription(NodeSubtreeSubscriptionParams&& _params)
{
    auto subscription = std::make_shared<impl::NodeSubtreeSubscriptionImpl>(std::move(_params));
    return subscription->Start() ? subscription : nullptr;
}

} // namespace xsdk::xnode
