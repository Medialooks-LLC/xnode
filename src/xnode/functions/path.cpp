#include "../xkey_type_match.h"
#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

INode::SPtr xnode::NodeGet(const INode::SPtr&             _node_this,
                           XPath&&                        _path,
                           std::optional<INode::NodeType> _node_type,
                           bool                           _convert_to_type)
{
    bool        create_nodes = _node_type.has_value();
    INode::SPtr node_dest    = _node_this;
    while (node_dest && _path.size() > 1) {
        auto key_node = _path.pop_front();
        node_dest     = xnode::NodeGetByKey(node_dest,
                                        key_node,
                                        create_nodes ? XKeyToNodeType::Match(_path.front()) : std::nullopt,
                                        false);
    }

    if (!node_dest || _path.empty())
        return node_dest;

    return xnode::NodeGetByKey(node_dest, _path.back(), _node_type, _convert_to_type);
}

INode::SPtrC xnode::NodeConstGet(const INode::SPtrC& _node_this, XPath&& _path)
{
    INode::SPtrC node_dest = _node_this;
    while (node_dest && !_path.empty())
        node_dest = node_dest->At(_path.pop_front()).QueryPtrC<INode>();

    return node_dest;
}

INode::SPtrC xnode::NodesMerge(const INode::SPtrC& _from, const INode::SPtrC& _to)
{
    if (!_from || !_to)
        return _from ? _from : _to;

    auto dest_node = xnode::Clone(_to, true);
    xnode::CopyTo(_from, dest_node, true);
    return dest_node;
}

std::optional<size_t> xnode::NodeSize(const INode::SPtrC& _node_this, XPath&& _path)
{
    auto dest_node = NodeConstGet(_node_this, std::move(_path));
    if (!dest_node)
        return std::nullopt;

    return dest_node->Size();
}

XValueRT xnode::At(const INode::SPtr& _node_this, XPath&& _path)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->At(key_dest);
}

XValueRT xnode::At(const INode::SPtrC& _node_this, XPath&& _path)
{
    INode::SPtrC node_dest = _node_this;
    auto         key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeConstGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->At(key_dest);
}

std::pair<bool, XValueRT> xnode::Set(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return {};

    return node_dest->Set(key_dest, std::move(_val));
}
INode::InsertRes xnode::Insert(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return {};

    return node_dest->Insert(key_dest, std::move(_val));
}

XValueRT xnode::Erase(const INode::SPtr& _node_this, XPath&& _path)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->Erase(key_dest);
}

XValueRT xnode::Increment(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.pop_back();
    if (!_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->Increment(key_dest, _val);
}

size_t xnode::EmplaceToArray(const INode::SPtr& _node_this, XPath&& _array_path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _array_path.pop_back();
    if (!_array_path.empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_array_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return 0;

    auto existed_val = node_dest->At(key_dest);
    auto array_node  = xnode::NodeGetByKey(_node_this, key_dest, INode::NodeType::Array, true);
    if (existed_val && existed_val != array_node) {
        assert(array_node->Empty());
        array_node->Insert(kIdxEnd, std::move(existed_val));
    }

    array_node->Insert(kIdxEnd, std::move(_val));
    return array_node->Size();
}

std::vector<XValueRT> xnode::ValuesList(const INode::SPtrC& _node_this, XPath&& _path)
{
    auto array_or_val = At(_node_this, std::move(_path));
    if (!array_or_val)
        return {};

    std::vector<XValueRT> result;
    auto                  node = array_or_val.QueryPtrC<INode>();
    if (node && node->Type() == INode::NodeType::Array) {
        node->BulkGetAll([&](const auto& key, const auto& val) {
            result.push_back(val);
            return OnCopyRes::Skip;
        });
    }
    else if (!node) {
        result.push_back(array_or_val);
    }

    return result;
}

std::vector<std::pair<XKey, XValueRT>> xnode::NodesList(const INode::SPtr& _node_this,
                                                        bool               _include_const,
                                                        XPath&&            _path)
{
    INode::SPtr node_target = xnode::NodeGet(_node_this, std::move(_path));
    if (!node_target)
        return {};

    return node_target->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        if ((_include_const && val.QueryPtrC<INode>()) || val.QueryPtr<INode>())
            return OnCopyRes::Take;
        return OnCopyRes::Skip;
    });
}

std::vector<std::pair<XKey, XValueRT>> xnode::NodesConstList(const INode::SPtrC& _node_this, XPath&& _path)
{
    INode::SPtrC node_target = xnode::NodeConstGet(_node_this, std::move(_path));
    if (!node_target)
        return {};

    return node_target->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        if (val.QueryPtrC<INode>())
            return OnCopyRes::Take;
        return OnCopyRes::Skip;
    });
}

std::vector<INode::SPtr> xnode::ChildNodesGet(
    const INode::SPtr&                                                            _node_this,
    XPath&&                                                                       _path,
    const bool                                                                    _child_nodes_unwrap,
    std::function<std::optional<bool>(const INode::SPtr& _node, size_t _taken)>&& _on_node_pf)
{
    INode::SPtr node_target = xnode::NodeGet(_node_this, std::move(_path));
    if (!node_target)
        return {};

    bool unwrap = _child_nodes_unwrap && (node_target->Type() == INode::NodeType::Array);

    std::vector<INode::SPtr> result;
    node_target->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        auto node = val.QueryPtr<INode>();
        if (!node)
            return OnCopyRes::Skip;

        auto cb_res = _on_node_pf ? _on_node_pf(node, result.size()) : std::optional<bool> {true};
        if (!cb_res.value_or(false))
            return cb_res.has_value() ? OnCopyRes::Skip : OnCopyRes::Stop;

        if (unwrap && node->NameGet().empty() && node->Size() == 1) {
            auto child_node = node->At(0).QueryPtr<INode>();
            if (child_node && !child_node->NameGet().empty()) {
                result.push_back(std::move(child_node));
                return OnCopyRes::Skip;
            }
        }

        result.push_back(std::move(node));
        return OnCopyRes::Skip;
    });

    return result;
}

std::vector<INode::SPtrC> xnode::ChildNodesConstGet(
    const INode::SPtrC&                                                            _node_this,
    XPath&&                                                                        _path,
    const bool                                                                     _child_nodes_unwrap,
    std::function<std::optional<bool>(const INode::SPtrC& _node, size_t _taken)>&& _on_node_pf)
{
    INode::SPtrC node_target = xnode::NodeConstGet(_node_this, std::move(_path));
    if (!node_target)
        return {};

    bool unwrap = _child_nodes_unwrap && (node_target->Type() == INode::NodeType::Array);

    std::vector<INode::SPtrC> result;
    node_target->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        auto node = val.QueryPtrC<INode>();
        if (!node)
            return OnCopyRes::Skip;

        auto cb_res = _on_node_pf ? _on_node_pf(node, result.size()) : std::optional<bool> {true};
        if (!cb_res.value_or(false))
            return cb_res.has_value() ? OnCopyRes::Skip : OnCopyRes::Stop;

        if (unwrap && node->NameGet().empty() && node->Size() == 1) {
            auto child_node = node->At(0).QueryPtrC<INode>();
            if (child_node && !child_node->NameGet().empty()) {
                result.push_back(std::move(child_node));
                return OnCopyRes::Skip;
            }
        }

        result.push_back(std::move(node));
        return OnCopyRes::Skip;
    });

    return result;
}

} // namespace xsdk
