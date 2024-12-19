#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

INode::SPtr xnode::Clone(
    XValue&&                                                                     _value_with_node,
    bool                                                                         _clone_nodes,
    const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>& _pf_on_item /*= nullptr*/,
    std::optional<std::string_view>                                              _cloned_name /*= {}*/,
    uint64_t                                                                     _cloned_uid /*= 0*/)
{
    std::vector<std::pair<XKey, XValue>> cloned_values;

    bool            have_nodes = false;
    std::string     node_name;
    INode::NodeType node_type = {};
    auto            node_p    = _value_with_node.QueryPtr<INode>();
    if (node_p) {
        node_p->BulkGetAll([&](const auto& key, const auto& value_rt) {
            auto value_for_change = value_rt;
            auto cb_res           = _pf_on_item ? _pf_on_item(node_p, key, value_for_change) : OnCopyRes::Take;
            if (cb_res == OnCopyRes::TakeStop || cb_res == OnCopyRes::Take) {
                cloned_values.emplace_back(key, value_for_change);

                if (!have_nodes && value_for_change.IsObject())
                    have_nodes = true;
            }

            return (cb_res == OnCopyRes::Stop || cb_res == OnCopyRes::TakeStop) ? OnCopyRes::Stop : OnCopyRes::Skip;
        });

        node_type = node_p->Type();
        node_name = node_p->NameGet();
    }
    else {
        auto node_cp = _value_with_node.QueryPtrC<INode>();
        if (!node_cp)
            return nullptr;

        node_cp->BulkGetAll([&](const auto& key, const auto& value_rt) {
            auto value_for_change = value_rt;
            auto cb_res           = _pf_on_item ? _pf_on_item(node_cp, key, value_for_change) : OnCopyRes::Take;
            if (cb_res == OnCopyRes::TakeStop || cb_res == OnCopyRes::Take) {
                cloned_values.emplace_back(key, value_for_change);

                if (!have_nodes && value_for_change.IsObject())
                    have_nodes = true;
            }

            return (cb_res == OnCopyRes::Stop || cb_res == OnCopyRes::TakeStop) ? OnCopyRes::Stop : OnCopyRes::Skip;
        });

        node_type = node_cp->Type();
        node_name = node_cp->NameGet();
    }

    if (have_nodes) {
        for (auto& [key, value] : cloned_values) {
            auto node_for_clone = value.QueryPtr<INode>();
            if (node_for_clone && _clone_nodes)
                value = XValueRT(xnode::Clone(node_for_clone, _clone_nodes, _pf_on_item, {}, 0));
            else if (node_for_clone)
                value = INode::SPtrC(node_for_clone); // Make const node
        }
    }

    auto cloned_p = xnode::Create(node_type, _cloned_name.value_or(node_name), _cloned_uid);
    assert(cloned_p);
    if (cloned_p)
        cloned_p->BulkInsert(std::move(cloned_values));

    return cloned_p;
}

size_t xnode::CopyTo(const INode::SPtrC& _source,
                     const INode::SPtr&  _dest,
                     bool                _override,
                     bool                _nodes_as_refs,
                     size_t              _depth)
{
    if (!_source || !_dest || _source == _dest)
        return {};

    assert(_source->Type() == _dest->Type());

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