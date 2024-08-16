#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

INode::SPtr xnode::Clone(
    XValue&&                                                                     _value_with_node,
    bool                                                                         _clone_nodes,
    const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>& _pf_on_item /*= nullptr*/,
    std::string_view                                                             _cloned_name /*= {}*/,
    uint64_t                                                                     _cloned_uid /*= 0*/)
{
    std::vector<std::pair<XKey, XValue>> cloned_values;

    bool have_nodes = false;
    INode::NodeType node_type  = {};
    auto node_p     = _value_with_node.QueryPtr<INode>();
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

    auto cloned_p = xnode::Create(node_type, _cloned_name, _cloned_uid);
    assert(cloned_p);
    if (cloned_p)
        cloned_p->BulkInsert(std::move(cloned_values));

    return cloned_p;
}

} // namespace xsdk