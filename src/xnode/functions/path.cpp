#include "../xkey_type_match.h"
#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

std::pair<INode::SPtr, size_t> xnode::ComplexCopyTo(std::vector<std::pair<XPath, XValue>>&& _values,
                                                    XValue&&                                _dest_value,
                                                    const CopyToFlags                       _copy_flags,
                                                    const std::string_view                  _default_new_node_name)
{
    auto node_dest = _dest_value.QueryPtr<INode>();
    if (!node_dest) {

        if (!xenum::HasFlag(_copy_flags, CopyToFlags::kCreateNew))
            return {};

        if (!_dest_value.IsEmpty() && !xenum::HasFlag(_copy_flags, CopyToFlags::kOverride))
            return {};

        // Check for const node
        auto const_node = _dest_value.QueryPtrC<INode>();
        if (const_node)
            node_dest = xnode::Clone(const_node, false, {}, _default_new_node_name);
        else
            node_dest = xnode::CreateMap({}, _dest_value.String(_default_new_node_name));
    }
    else if (xenum::HasFlag(_copy_flags, CopyToFlags::kCreateOverrideAll) && !_default_new_node_name.empty()) {
        node_dest->NameSet(_dest_value.String(_default_new_node_name), true);
    }

    size_t applied = 0;
    for (auto& [path, val] : _values) {
        if (xenum::HasFlag(_copy_flags, CopyToFlags::kOverride))
            applied += xnode::Set(node_dest, std::move(path), std::move(val)).first;
        else
            applied += xnode::Insert(node_dest, std::move(path), std::move(val)).succeeded;
    }

    return {node_dest, applied};
}

INode::SPtr xnode::NodeGet(const INode::SPtr&                   _node_this,
                           XPath&&                              _path,
                           const std::optional<INode::NodeType> _node_type,
                           const bool                           _convert_to_type)
{
    bool        create_nodes = _node_type.has_value();
    INode::SPtr node_dest    = _node_this;
    while (node_dest && _path.Size() > 1) {
        auto key_node = _path.PopFront();
        node_dest     = xnode::NodeGetByKey(node_dest,
                                        key_node,
                                        create_nodes ? XKeyToNodeType::Match(_path.At(0)) : std::nullopt,
                                        false);
    }

    if (!node_dest || _path.Empty())
        return node_dest;

    return xnode::NodeGetByKey(node_dest, _path.Back(), _node_type, _convert_to_type);
}

INode::SPtr xnode::NodeGetV(const XValue&                        _node_value,
                            XPath&&                              _path,
                            const std::optional<INode::NodeType> _node_type,
                            const bool                           _convert_to_type)
{
    return xnode::NodeGet(_node_value.QueryPtr<INode>(), std::move(_path), _node_type, _convert_to_type);
}

INode::SPtrC xnode::NodeConstGet(const INode::SPtrC& _node_this, XPath&& _path)
{
    INode::SPtrC node_dest = _node_this;
    while (node_dest && !_path.Empty())
        node_dest = node_dest->At(_path.PopFront()).QueryPtrC<INode>();

    return node_dest;
}

INode::SPtrC xnode::NodeConstGetV(const XValue& _node_value, XPath&& _path)
{
    return xnode::NodeConstGet(_node_value.QueryPtrC<INode>(), std::move(_path));
}

INode::SPtrC xnode::NodesMerge(const INode::SPtrC& _from, const INode::SPtrC& _to)
{
    if (!_from || !_to || _from == _to)
        return _from ? _from : _to;

    auto dest_node = xnode::Clone(_to, true);
    xnode::PatchApply(dest_node, _from);
    return dest_node;
}

XValue xnode::ValuePatch(const XValue& _patch, const XValue& _dest, const bool _modify_dest_node)
{
    if (_patch.IsEmpty() || _patch.Compare(_dest) == 0)
        return _dest;

    auto node_patch = _patch.QueryPtrC<INode>();
    auto node_dest  = _dest.QueryPtrC<INode>();
    if (!node_patch || !node_dest)
        return _patch.Type() == XValue::kNull ? XValue() : _patch;

    // Do not use NodesMerge for ability to return non-const INode
    auto patch_dest = _modify_dest_node ? _dest.QueryPtr<INode>() : nullptr;
    if (!patch_dest)
        patch_dest = xnode::Clone(node_dest, true);
    xnode::PatchApply(patch_dest, node_patch);
    return patch_dest;
}

std::optional<size_t> xnode::NodeSize(const INode::SPtrC& _node_this, XPath&& _path)
{
    auto dest_node = NodeConstGet(_node_this, std::move(_path));
    if (!dest_node)
        return std::nullopt;

    return dest_node->Size();
}

std::pair<XPath, INode::SPtrC> xnode::NodePath(const INode* _node_this_p, const INode* _root_p)
{

    XPath path        = {};
    auto  target_node = xobject::PtrQuery<INode>(_node_this_p);
    while (target_node) {
        auto parent = target_node->ParentGet();
        if (!parent || parent.get() == _root_p)
            break;

        assert(parent->Type() != INode::NodeType::Array);
        assert(!target_node->NameGet().empty());
        path.PushFront(target_node->NameGet());
        target_node = parent;
    }

    return {path, target_node};
}

size_t xnode::NodeIterate(const XValue&                                             _node_value,
                          const std::function<bool(const XPath&, const XValueRT&)>& _on_each_item,
                          const XPath&                                              _node_path)
{
    auto node_this = _node_value.QueryPtrC<INode>();
    if (!node_this)
        return 0;

    size_t counter = 0;
    XPath  item_path(_node_path);
    item_path.PushBack({});
    node_this->ForPatch([&](const XKey& _key, const XValueRT& _val) -> bool {
        item_path.Back() = _key;
        if (!_on_each_item(item_path, _val))
            return true;

        ++counter;

        counter += xnode::NodeIterate(_val, _on_each_item, item_path);
        return false;
    });

    return counter;
}

XValueRT xnode::ChangesAfterTime(const XValueRT&     _value,
                                 const xbase::Time64 _after_timestamp,
                                 const bool          _unwrap_const_nodes)
{
    auto node_sp = _value.QueryPtrC<INode>();
    if (!node_sp)
        return {_value.Timestamp() > _after_timestamp ? _value : XValueRT {}};

    // For better perfomance do not unwrap (if not specified) const nodes.
    if (!_unwrap_const_nodes && !_value.QueryPtr<INode>())
        return {_value.Timestamp() > _after_timestamp ? _value : XValueRT {}};

    XValueRT      mod_value;
    xbase::Time64 mod_time = time64::kPast;
    node_sp->ForPatch([&](const XKey& _key, const XValueRT& _val) -> bool {
        XValueRT changed_val;
        // Check for removed value (changed_val is in Empty (monostate) in this case and next if is false)
        if (_val)
            changed_val = xnode::ChangesAfterTime(_val, _after_timestamp, _unwrap_const_nodes);
        else if (_val.Timestamp() > _after_timestamp)
            changed_val = XValueRT(nullptr, _val.Timestamp());

        // If value or subnode not modified -> continue;
        if (!changed_val)
            return false;

        mod_time = std::max(mod_time, changed_val.Timestamp());
        xnode::NodeSet(mod_value, XPath(_key), std::move(changed_val));

        return false;
    });

    return {(XValue)mod_value, mod_time};
}

XValueRT xnode::At(const INode::SPtr& _node_this, XPath&& _path)
{
    return xnode::At(INode::SPtrC {_node_this}, std::move(_path));
}

XValueRT xnode::At(const INode::SPtrC& _node_this, XPath&& _path)
{
    if (_path.Empty())
        return _node_this;

    INode::SPtrC node_dest = _node_this;
    auto         key_dest  = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeConstGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->At(key_dest);
}

XValueRT xnode::At(const std::vector<INode::SPtrC>& _check_nodes, const XPath& _path)
{
    for (const auto& node : _check_nodes) {
        auto val = xnode::At(node, XPath(_path));
        if (val)
            return val;
    }

    return {};
}

XValueRT xnode::At(const XValue& _node_value, XPath&& _path)
{
    if (_path.Empty())
        return _node_value;

    return xnode::At(_node_value.QueryPtrC<INode>(), std::move(_path));
}

std::pair<bool, XValueRT> xnode::Set(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return {};

    return node_dest->Set(key_dest, std::move(_val));
}

std::pair<bool, XValueRT> xnode::NodeSet(XValueRT& _node_value, XPath&& _path, XValue&& _val)
{
    assert(!_path.Empty());
    if (_path.Empty())
        return {};

    auto        node_type = XKeyToNodeType::Match(_path.Front()).value_or(INode::NodeType::Map);
    INode::SPtr node_dest = _node_value.QueryPtr<INode>();
    if (!node_dest || node_dest->Type() != node_type) {
        node_dest   = xnode::Create(node_type);
        _node_value = XValueRT(node_dest);
    }

    auto key_dest = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeGet(node_dest, std::move(_path), XKeyToNodeType::Match(key_dest), true);

    if (!node_dest)
        return {};

    return node_dest->Set(key_dest, std::move(_val));
}

INode::InsertRes xnode::Insert(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return {};

    return node_dest->Insert(key_dest, std::move(_val));
}

XValueRT xnode::Erase(const INode::SPtr& _node_this, XPath&& _path)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path));

    if (!node_dest)
        return {};

    return node_dest->Erase(key_dest);
}

XValueRT xnode::Increment(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _path.PopBack();
    if (!_path.Empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_path), XKeyToNodeType::Match(key_dest), true);

    if (!node_dest)
        return {};

    return node_dest->Increment(key_dest, _val);
}

size_t xnode::EmplaceToArray(const INode::SPtr& _node_this, XPath&& _array_path, XValue&& _val)
{
    INode::SPtr node_dest = _node_this;
    auto        key_dest  = _array_path.PopBack();
    if (!_array_path.Empty())
        node_dest = xnode::NodeGet(_node_this, std::move(_array_path), XKeyToNodeType::Match(key_dest));

    if (!node_dest)
        return 0;

    auto existed_val = node_dest->At(key_dest);
    auto array_node  = xnode::NodeGetByKey(node_dest, key_dest, INode::NodeType::Array, true);
    if (existed_val && existed_val != array_node) {
        assert(array_node->Empty());
        array_node->Insert(kIdxEnd, std::move(existed_val));
    }

    array_node->Insert(kIdxEnd, std::move(_val));
    return array_node->Size();
}

std::vector<XValueRT> xnode::ValuesList(const XValue&                        _target_value,
                                        XPath&&                              _path,
                                        const std::optional<INode::NodeType> _only_for_type)
{
    auto array_or_val = At(_target_value, std::move(_path));
    if (!array_or_val)
        return {};

    std::vector<XValueRT> result;
    auto                  node = array_or_val.QueryPtrC<INode>();
    if (node && (!_only_for_type.has_value() || node->Type() == _only_for_type.value())) {
        node->BulkGetAll([&](const auto& key, const auto& val) {
            result.push_back(val);
            return OnCopyRes::Skip;
        });
    }
    else if (!node && !_only_for_type.has_value()) {
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
