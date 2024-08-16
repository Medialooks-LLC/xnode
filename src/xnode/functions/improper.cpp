#include "xnode_functions.h"

namespace xsdk {

std::map<XValueRT, XValueRT> xnode::ParentsCheck(const XValue&                  _root,
                                                     bool                           _include_const,
                                                     std::map<XValueRT, XValueRT>&& _improper_map)
{
    INode::SPtrC node_root_c = _root.QueryPtrC<INode>();
    if (!node_root_c)
        return std::move(_improper_map);

    std::vector<std::pair<XKey, XValueRT>> childs;
    auto                                   node_p = _root.QueryPtr<INode>();
    if (node_p)
        childs = xnode::NodesList(node_p, _include_const);
    else
        childs = xnode::NodesConstList(node_root_c);

    // check nodes
    for (const auto& [key, val] : childs) {
        auto node_const_p = val.QueryPtrC<INode>();
        assert(node_const_p);
        if (_improper_map.count(val) > 0)
            continue; // find cicle !!!

        if (node_const_p->ParentGet() != node_root_c)
            _improper_map.emplace(val, _root);

        _improper_map = xnode::ParentsCheck(val, _include_const, std::move(_improper_map));
    }

    return std::move(_improper_map);
}

std::map<XValueRT, XValueRT> xnode::ParentsFix(const std::map<XValueRT, XValueRT>& _fix_map)
{
    std::map<XValueRT, XValueRT> fixed;
    for (auto& [child, parent] : _fix_map) {
        auto node_parent = parent.QueryPtr<INode>();
        auto node_child  = parent.QueryPtr<INode>();
        if (node_parent && node_child)
            node_child->ParentSet(node_parent);

        fixed.emplace(child, parent);
    }

    return fixed;
}
} // namespace xsdk