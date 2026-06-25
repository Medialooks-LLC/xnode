#include "xnode.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

using namespace xsdk;

// NOLINTBEGIN(*)

namespace {

class SaveCounter {
public:
    std::error_code Save(const INode::SPtrC&)
    {
        const std::unique_lock lck(mutex_);
        ++count_;
        cv_.notify_all();
        return {};
    }

    bool WaitFor(const int _count, const std::chrono::milliseconds _timeout = std::chrono::milliseconds(1000))
    {
        std::unique_lock lck(mutex_);
        return cv_.wait_for(lck, _timeout, [&] { return count_ >= _count; });
    }

    int Count() const
    {
        const std::unique_lock lck(mutex_);
        return count_;
    }

private:
    mutable std::mutex      mutex_;
    std::condition_variable cv_;
    int                     count_ = 0;
};

} // namespace

TEST(xnode_auto_save, subtree_subscription_tracks_existing_and_new_nodes)
{
    auto root     = xnode::Create(INode::NodeType::Map, "root");
    auto existing = xnode::Create(INode::NodeType::Map, "existing");
    ASSERT_TRUE(root->Set("existing", existing).first);

    std::atomic<int> changes {0};
    auto             subscription = xnode::CreateNodeSubtreeSubscription(
        {root,
         [&](INode::CallbackReason _reason, const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&) {
             if (_reason != INode::CallbackReason::Rollback)
                 ++changes;
         },
         true});
    ASSERT_TRUE(subscription);

    ASSERT_TRUE(existing->Set("value", 1).first);
    EXPECT_EQ(changes.load(), 1);

    auto added = xnode::Create(INode::NodeType::Map, "added");
    ASSERT_TRUE(root->Set("added", added).first);
    EXPECT_EQ(changes.load(), 2);

    ASSERT_TRUE(added->Set("value", 2).first);
    EXPECT_EQ(changes.load(), 3);

    subscription->Stop();
    ASSERT_TRUE(added->Set("value", 3).first);
    EXPECT_EQ(changes.load(), 3);
}

TEST(xnode_auto_save, subtree_subscription_detaches_removed_subtree)
{
    const auto root       = xnode::Create(INode::NodeType::Map, "root");
    const auto child      = xnode::Create(INode::NodeType::Map, "child");
    const auto grandchild = xnode::Create(INode::NodeType::Map, "grandchild");
    ASSERT_TRUE(child->Set("grandchild", grandchild).first);
    ASSERT_TRUE(root->Set("child", child).first);

    std::atomic<int> changes {0};
    const auto       subscription = xnode::CreateNodeSubtreeSubscription(
        {root,
         [&](INode::CallbackReason _reason, const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&) {
             if (_reason != INode::CallbackReason::Rollback)
                 ++changes;
         },
         true});
    ASSERT_TRUE(subscription);

    ASSERT_TRUE(child->Set("before", 1).first);
    EXPECT_EQ(changes.load(), 1);

    const auto removed = root->Erase("child");
    ASSERT_TRUE(removed.QueryPtr<INode>());
    EXPECT_EQ(changes.load(), 2);

    ASSERT_TRUE(child->Set("after", 2).first);
    ASSERT_TRUE(grandchild->Set("after", 3).first);

    const auto added_after_detach = xnode::Create(INode::NodeType::Map, "added_after_detach");
    ASSERT_TRUE(child->Set("added_after_detach", added_after_detach).first);
    ASSERT_TRUE(added_after_detach->Set("after", 4).first);

    EXPECT_EQ(changes.load(), 2);
}

TEST(xnode_auto_save, subtree_subscription_keeps_renamed_subtree)
{
    const auto root       = xnode::Create(INode::NodeType::Map, "root");
    const auto child      = xnode::Create(INode::NodeType::Map, "child");
    const auto grandchild = xnode::Create(INode::NodeType::Map, "grandchild");
    ASSERT_TRUE(child->Set("grandchild", grandchild).first);
    ASSERT_TRUE(root->Set("child", child).first);

    std::atomic<int> changes {0};
    const auto       subscription = xnode::CreateNodeSubtreeSubscription(
        {root,
         [&](INode::CallbackReason _reason, const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&) {
             if (_reason != INode::CallbackReason::Rollback)
                 ++changes;
         },
         true});
    ASSERT_TRUE(subscription);

    const auto before_rename = changes.load();
    const auto rename_res    = child->NameSet("renamed_child", true);
    ASSERT_TRUE(rename_res.first);
    EXPECT_GT(changes.load(), before_rename);

    const auto before_nested_change = changes.load();
    ASSERT_TRUE(grandchild->Set("after_rename", 1).first);
    EXPECT_EQ(changes.load(), before_nested_change + 1);
}

TEST(xnode_auto_save, coalesces_changes_with_debounce)
{
    const auto root = xnode::Create(INode::NodeType::Map, "root");

    SaveCounter counter;
    const auto  auto_save = xnode::CreateNodeAutoSave({root,
                                                       [&](const INode::SPtrC& _node) { return counter.Save(_node); },
                                                       200 * time64::kMsec,
                                                       1000 * time64::kMsec});
    ASSERT_TRUE(auto_save);

    ASSERT_TRUE(root->Set("a", 1).first);
    ASSERT_TRUE(root->Set("b", 2).first);
    ASSERT_TRUE(root->Set("c", 3).first);

    ASSERT_TRUE(counter.WaitFor(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_EQ(counter.Count(), 1);
}

TEST(xnode_auto_save, continues_saving_with_zero_debounce)
{
    const auto root = xnode::Create(INode::NodeType::Map, "root");

    SaveCounter counter;
    const auto  auto_save = xnode::CreateNodeAutoSave(
        {root, [&](const INode::SPtrC& _node) { return counter.Save(_node); }, 0, 0});
    ASSERT_TRUE(auto_save);

    ASSERT_TRUE(root->Set("first", 1).first);
    ASSERT_TRUE(counter.WaitFor(1));

    ASSERT_TRUE(root->Set("second", 2).first);
    ASSERT_TRUE(counter.WaitFor(2));
}

TEST(xnode_auto_save, flush_saves_immediately)
{
    const auto root = xnode::Create(INode::NodeType::Map, "root");

    SaveCounter counter;
    const auto  auto_save = xnode::CreateNodeAutoSave({root,
                                                       [&](const INode::SPtrC& _node) { return counter.Save(_node); },
                                                       1000 * time64::kMsec,
                                                       1000 * time64::kMsec});
    ASSERT_TRUE(auto_save);

    ASSERT_TRUE(root->Set("value", 1).first);
    auto_save->Flush();

    EXPECT_TRUE(counter.WaitFor(1));
    EXPECT_EQ(counter.Count(), 1);
}

TEST(xnode_auto_save, flush_waits_for_running_scheduled_save)
{
    const auto root = xnode::Create(INode::NodeType::Map, "root");

    std::promise<void> save_started_promise;
    auto               save_started = save_started_promise.get_future();

    std::promise<void> release_save_promise;
    auto               release_save = release_save_promise.get_future().share();

    std::atomic<bool> save_started_once {false};
    std::atomic<bool> save_finished {false};
    auto              save = [&](const INode::SPtrC&) {
        if (!save_started_once.exchange(true))
            save_started_promise.set_value();

        release_save.wait();
        save_finished = true;
        return std::error_code {};
    };
    const auto auto_save = xnode::CreateNodeAutoSave({root, save, 0, 0});
    ASSERT_TRUE(auto_save);

    ASSERT_TRUE(root->Set("value", 1).first);
    ASSERT_EQ(save_started.wait_for(std::chrono::seconds(1)), std::future_status::ready);

    auto flush_done = std::async(std::launch::async, [&] { auto_save->Flush(); });
    EXPECT_EQ(flush_done.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
    EXPECT_FALSE(save_finished.load());

    release_save_promise.set_value();
    EXPECT_EQ(flush_done.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_TRUE(save_finished.load());
}

TEST(xnode_auto_save, rollback_does_not_leave_pending_save)
{
    const auto root = xnode::Create(INode::NodeType::Map, "root");

    SaveCounter counter;
    const auto  auto_save = xnode::CreateNodeAutoSave({root,
                                                       [&](const INode::SPtrC& _node) { return counter.Save(_node); },
                                                       1000 * time64::kMsec,
                                                       1000 * time64::kMsec});
    ASSERT_TRUE(auto_save);

    const auto veto_uid = root->OnChangeAdd(
        [](const INode::CallbackReason _reason, const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&) {
            return _reason == INode::CallbackReason::Changes ? std::optional<bool>(false) : std::optional<bool>(true);
        });
    ASSERT_NE(veto_uid, 0);

    EXPECT_FALSE(root->Set("blocked", 1).first);
    auto_save->Flush();

    EXPECT_EQ(counter.Count(), 0);
    EXPECT_TRUE(root->OnChangeRemove(veto_uid));
}

TEST(xnode_auto_save, stop_can_discard_or_flush_pending_save)
{
    {
        const auto root = xnode::Create(INode::NodeType::Map, "discard");

        SaveCounter counter;
        const auto  auto_save = xnode::CreateNodeAutoSave(
            {root,
             [&](const INode::SPtrC& _node) { return counter.Save(_node); },
             200 * time64::kMsec,
             200 * time64::kMsec});
        ASSERT_TRUE(auto_save);

        ASSERT_TRUE(root->Set("value", 1).first);
        auto_save->Stop(false);
        auto_save->Flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(260));
        EXPECT_EQ(counter.Count(), 0);
    }

    {
        const auto root = xnode::Create(INode::NodeType::Map, "flush");

        SaveCounter counter;
        const auto  auto_save = xnode::CreateNodeAutoSave(
            {root,
             [&](const INode::SPtrC& _node) { return counter.Save(_node); },
             200 * time64::kMsec,
             200 * time64::kMsec});
        ASSERT_TRUE(auto_save);

        ASSERT_TRUE(root->Set("value", 1).first);
        auto_save->Stop(true);
        EXPECT_TRUE(counter.WaitFor(1));
        EXPECT_EQ(counter.Count(), 1);
    }
}

TEST(xnode_auto_save, saves_changes_from_nested_nodes)
{
    const auto root  = xnode::Create(INode::NodeType::Map, "root");
    const auto child = xnode::Create(INode::NodeType::Map, "child");
    ASSERT_TRUE(root->Set("child", child).first);

    SaveCounter counter;
    const auto  auto_save = xnode::CreateNodeAutoSave({root,
                                                       [&](const INode::SPtrC& _node) { return counter.Save(_node); },
                                                       30 * time64::kMsec,
                                                       300 * time64::kMsec});
    ASSERT_TRUE(auto_save);

    ASSERT_TRUE(child->Set("value", 1).first);
    ASSERT_TRUE(counter.WaitFor(1));

    const auto added = xnode::Create(INode::NodeType::Map, "added");
    ASSERT_TRUE(root->Set("added", added).first);
    ASSERT_TRUE(counter.WaitFor(2));

    ASSERT_TRUE(added->Set("value", 2).first);
    ASSERT_TRUE(counter.WaitFor(3));
}

// NOLINTEND(*)
