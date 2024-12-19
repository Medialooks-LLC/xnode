#include "xnode_functions.h"

namespace xsdk {

INode::InsertRes xnode::ArrayInsertWrapped(const INode::SPtr& _node_array, XValue&& _val, const size_t _insert_at)
{
    if (!_node_array || _node_array->Type() != INode::NodeType::Array)
        return {false, {}};

    auto node = _val.QueryPtrC<INode>();
    if (node && !node->NameGet().empty())
        return _node_array->Insert(_insert_at, xnode::CreateMap({{node->NameGet(), std::move(_val)}}));

    return _node_array->Insert(_insert_at, std::move(_val));
}

XValue xnode::ArrayNodesWrap(XValue&& _array_val)
{
    auto array_node = _array_val.QueryPtrC<INode>();
    if (!array_node || array_node->Type() != INode::NodeType::Array)
        return std::move(_array_val);

    INode::SPtr updated_node;
    auto        child_nodes = xnode::NodesConstList(array_node);
    for (auto& [key, xval] : child_nodes) {
        auto node_child = xval.QueryPtrC<INode>();
        assert(node_child);
        if (!node_child->NameGet().empty()) {
            if (!updated_node)
                updated_node = xnode::Clone(std::move(_array_val), false);

            updated_node->Set(key, xnode::CreateMap({{node_child->NameGet(), node_child}}));
        }
    }

    if (updated_node)
        return updated_node;

    return std::move(_array_val);
}

XValue xnode::ArrayNodesUnwrap(XValue&& _array_val)
{
    auto array_node = _array_val.QueryPtrC<INode>();
    if (!array_node || array_node->Type() != INode::NodeType::Array)
        return _array_val;

    INode::SPtr updated_node;
    auto        child_nodes = xnode::NodesConstList(array_node);
    for (auto& [key, xval] : child_nodes) {
        auto node_child = xval.QueryPtrC<INode>();
        assert(node_child);
        if (node_child->Size() != 1)
            continue;

        auto node_unwrap = node_child->At(0).QueryPtrC<INode>();
        if (node_unwrap) {
            if (!updated_node)
                updated_node = xnode::Clone(std::move(_array_val), false);

            updated_node->Set(key, node_unwrap);
        }
    }

    if (updated_node)
        return updated_node;

    return std::move(_array_val);
}

std::map<XValueRT, XValueRT> xnode::utility::ParentsCheck(const XValue&                  _root,
                                                          bool                           _include_const,
                                                          std::map<XValueRT, XValueRT>&& _improper_map)
{
    INode::SPtrC node_root_c = _root.QueryPtrC<INode>();
    if (!node_root_c)
        return std::move(_improper_map);

    std::vector<std::pair<XKey, XValueRT>> childs;

    auto node_p = _root.QueryPtr<INode>();
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

        _improper_map = xnode::utility::ParentsCheck(val, _include_const, std::move(_improper_map));
    }

    return std::move(_improper_map);
}

std::map<XValueRT, XValueRT> xnode::utility::ParentsFix(const std::map<XValueRT, XValueRT>& _fix_map)
{
    std::map<XValueRT, XValueRT> fixed;
    for (auto& [child, parent] : _fix_map) {
        auto node_parent = parent.QueryPtr<INode>();
        auto node_child  = child.QueryPtr<INode>();
        if (node_parent && node_child)
            node_child->ParentSet(node_parent);

        fixed.emplace(child, parent);
    }

    return fixed;
}
} // namespace xsdk