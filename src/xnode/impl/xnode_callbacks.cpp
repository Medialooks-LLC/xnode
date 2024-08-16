#include "xnode_callbacks.h"

namespace xsdk::impl {

uint64_t XNodeCallbacks::OnChangeAdd(INode::OnChangePF&& _pf_on_change, const uint64_t _id)
{
    if (!_pf_on_change)
        return 0;

    std::unique_lock lck(map_rw_);

    auto id = _id;
    if (id == 0 && !callbacks_map_.empty())
        id = std::max(callbacks_map_.rbegin()->first + 1, xbase::NextUid());
    else if (id == 0)
        id = xbase::NextUid();

    callbacks_map_[id] = _pf_on_change;
    return id;
}

bool XNodeCallbacks::OnChangeRemove(uint64_t _id)
{
    std::unique_lock lck(map_rw_);

    return callbacks_map_.erase(_id) > 0;
}

size_t XNodeCallbacks::OnChangeReset()
{
    std::unique_lock lck(map_rw_);

    return std::exchange(callbacks_map_, {}).size();
}

bool XNodeCallbacks::DoCallbacks(const INode::SPtrC& _node,
                                   const XKey&         _key,
                                   const XValueRT&    _from,
                                   const XValueRT&    _to,
                                   bool                _no_discard)
{
    std::shared_lock lck(map_rw_);

    std::vector<uint64_t> expired;
    uint64_t              failed_uid = 0;
    for (const auto& [uid, callback] : callbacks_map_) {
        assert(callback);
        auto res_opt = callback(_no_discard ? INode::CallbackReason::ChangesNoDiscard : INode::CallbackReason::Changes,
                                _node,
                                _key,
                                _from,
                                _to);
        if (!res_opt.has_value()) {
            expired.push_back(uid);
        }
        else if (!_no_discard && !res_opt.value()) {
            failed_uid = uid;
            break;
        }
    }

    if (failed_uid != 0) {
        assert(!_no_discard);
        // Rollback changes
        for (auto it = callbacks_map_.begin(); it != callbacks_map_.end() && it->first != failed_uid; ++it)
            it->second(INode::CallbackReason::Rollback, _node, _key, _to, _from);
    }

    lck.unlock();

    if (!expired.empty()) {
        // Remove expired
        std::unique_lock lck(map_rw_);
        for (auto uid : expired)
            callbacks_map_.erase(uid);
    }

    return failed_uid ? false : true;
}

} // namespace xsdk::impl