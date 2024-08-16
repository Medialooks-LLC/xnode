#pragma once

#include "xnode_interfaces.h"

#include <functional>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace xsdk::impl {

class XNodeCallbacks {
    std::shared_mutex                     map_rw_;
    std::map<uint64_t, INode::OnChangePF> callbacks_map_;

public:
    uint64_t OnChangeAdd(INode::OnChangePF&& _pf_on_change, const uint64_t _id = 0);
    bool     OnChangeRemove(uint64_t _id);
    size_t   OnChangeReset();

    bool DoCallbacks(const INode::SPtrC& _node,
                      const XKey&         _key,
                      const XValueRT&    _from,
                      const XValueRT&    _to,
                      bool                _no_discard);
};

} // namespace xsdk::impl