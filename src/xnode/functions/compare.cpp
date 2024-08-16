#include "xnode_functions.h"

#include <vector>

namespace xsdk {

int32_t xnode::Compare(
    const INode::SPtrC&                                                                            _node_left,
    const INode::SPtrC&                                                                            _node_right,
    bool                                                                                           _nodes_unwrap,
    const std::function<bool(const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&)>& _pf_on_different)
{
    if (!_node_left || !_node_right)
        return (int32_t)(_node_left.get() - _node_right.get());

    if (_node_left->Type() != _node_right->Type())
        return (int32_t)_node_left->Type() - (int32_t)_node_right->Type();

    // todo: optimize for arrays compare (w/o keys)
    int32_t compare_res  = 0;
    auto    values_right = _node_right->BulkGetAll(
        [](const XKey&, const XValueRT& val) { return val ? OnCopyRes::Take : OnCopyRes::Skip; });

    auto it_right = values_right.begin();

    std::vector<std::pair<INode::SPtrC, INode::SPtrC>> compare_nodes;

    _node_left->BulkGetAll([&](const XKey& key_left, const XValueRT& val_left) {
        if (it_right == values_right.end() || key_left < it_right->first) {
            if (!_pf_on_different || _pf_on_different(_node_left, key_left, val_left, XValue())) {
                compare_res = 1;
                return OnCopyRes::Stop;
            }

            return OnCopyRes::Skip;
        }

        if (key_left > it_right->first) {
            if (!_pf_on_different || _pf_on_different(_node_left, it_right->first, XValue(), it_right->second)) {
                compare_res = -1;
                return OnCopyRes::Stop;
            }
        }
        else if (val_left != it_right->second) {
            assert(key_left == it_right->first);
            auto val_compare = val_left.Compare(it_right->second);
            if (val_compare != 0) {
                // check for nodes
                auto node_left  = val_left.QueryPtrC<INode>();
                auto node_right = it_right->second.QueryPtrC<INode>();
                if (_nodes_unwrap && node_left && node_right && node_left->Type() == node_right->Type()) {
                    compare_nodes.emplace_back(node_left, node_right);
                }
                else if (!_pf_on_different || _pf_on_different(_node_left, key_left, val_left, it_right->second)) {
                    compare_res = val_compare;
                    return OnCopyRes::Stop;
                }
            }
        }

        ++it_right;

        return OnCopyRes::Skip;
    });

    if (compare_res != 0)
        return compare_res;

    if (it_right != values_right.end())
        return -1;

    for (const auto& [node_left, node_right] : compare_nodes) {
        compare_res = xnode::Compare(node_left, node_right, true, _pf_on_different);
        if (compare_res != 0)
            return compare_res;
    }

    return 0;
}

std::pair<size_t, size_t> xnode::PatchApply(const INode::SPtr& _target, const INode::SPtrC& _patch)
{
    if (!_target || !_patch || _target == _patch)
        return {};

    size_t added   = 0;
    size_t erased = 0;

    std::vector<std::pair<INode::SPtr, INode::SPtrC>> nodes_vec;
    _patch->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        if (val.Type() == XValue::ValueType::kNull) {
            if(_target->Erase(key))
                ++erased;
        }
        else {
            auto target_node = _target->At(key).QueryPtr<INode>();
            auto patch_node  = val.QueryPtrC<INode>();
            if (target_node && patch_node)
                nodes_vec.emplace_back(target_node, patch_node);
            else if (_target->Set(key, XValue(val)).first)
                ++added;
        }

        return OnCopyRes::Skip;
    });

    for (auto& [target, patch] : nodes_vec)
    {
        auto [added_nested, erased_nested] = PatchApply(target, patch);
        added += added_nested;
        erased += erased_nested;
    }

    return {added, erased};
}

size_t xnode::CopyTo(const INode::SPtrC& _source,
                     const INode::SPtr&  _dest,
                     bool                _override,
                     bool                _nodes_as_refs,
                     size_t              _depth)
{
    if (!_source || !_dest || _source == _dest)
        return {};

    size_t copied = 0;
    // Possible optimization: collect to vector and use BulkSet() (like in Clone)
    std::vector<std::pair<XKey, INode::SPtrC>>        nodes_for_copy;
    std::vector<std::pair<INode::SPtr, INode::SPtrC>> copy_next;
    _source->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        auto dest_value = _dest->At(key);
        if (dest_value && !_override)
            return OnCopyRes::Skip;

        auto dest_node = dest_value.QueryPtr<INode>();
        auto src_node  = val.QueryPtrC<INode>();
        if (dest_node && src_node) {
            if (_depth > 0)
                copy_next.emplace_back(dest_node, src_node);
        }
        else {
            if (src_node) {
                if (_nodes_as_refs) {
                    _dest->Set(key, src_node);
                    ++copied;
                }
                else {
                    nodes_for_copy.emplace_back(key, src_node);
                }
            }
            else if (_dest->Set(key, XValue(val)).first) {
                ++copied;
            }
        }
        return OnCopyRes::Skip;
    });

    for (auto& [key, src_node] : nodes_for_copy) {
        _dest->Set(key, xnode::Clone(src_node, true));
        copied += 1; // Could make better calculation
    }

    for (auto& [dest, src] : copy_next) {
        assert(_depth > 0);
        copied += CopyTo(src, dest, _depth - 1);
    }

    return copied;
}

} // namespace xsdk