#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

xnode::XNodeType xnode::NodeTypeGet(const XValue& _val)
{
    if (!_val.IsObject())
        return XNodeType::not_node;

    auto node_p = _val.QueryPtr<INode>();
    if (node_p)
        return node_p->Type() == INode::NodeType::Map ? XNodeType::map : XNodeType::array;

    auto node_cp = _val.QueryPtrC<INode>();
    if (node_cp)
        return node_cp->Type() == INode::NodeType::Map ? XNodeType::const_map : XNodeType::const_array;

    return XNodeType::not_node;
}

std::optional<size_t> xnode::NodeSizeV(const XValue& _value)
{
    auto node = _value.QueryPtrC<INode>();
    if (!node)
        return std::nullopt;

    return node->Size();
}

std::optional<INode::NodeType> xnode::NodeType(const XValue& _value)
{
    auto node = _value.QueryPtrC<INode>();
    if (!node)
        return std::nullopt;

    return node->Type();
}

INode::InsertRes xnode::NodeInsert(const INode::SPtr& _node_this,
                                   const INode::SPtr& _node_insert,
                                   bool               _replace_node,
                                   const XKey&        _node_key)
{
    if (!_node_this || !_node_insert)
        return {};

    XKey key = _node_key;
    if (!key)
        key = _node_this->Type() == INode::NodeType::Map ? XKey(_node_insert->NameGet()) : XKey(kIdxEnd);

    if (!_node_this->IsKeyValid(true, key))
        return {};

    if (!_replace_node || key.IndexGet().value_or(0) == kIdxEnd)
        return _node_this->Insert(key, _node_insert);

    // WARNING !!! kIdxLast not correct, also map indexes not corrected
    auto [updated, val] = _node_this->Set(key, _node_insert);
    return {updated, key, updated ? XValueRT() : val /*XValue(_node_insert)*/};
}

INode::InsertRes xnode::NodeConstInsert(const INode::SPtr&  _node_this,
                                        const INode::SPtrC& _node_insert,
                                        bool                _replace_node,
                                        const XKey&         _node_key)
{
    if (!_node_this)
        return {};

    XKey key = _node_key;
    if (!key)
        key = _node_this->Type() == INode::NodeType::Map ? XKey(_node_insert->NameGet()) : XKey((size_t)-1);

    if (!_node_this->IsKeyValid(true, key))
        return {};

    if (!_replace_node || key.IndexGet().value_or(0) == kIdxEnd)
        return _node_this->Insert(key, _node_insert);

    // WARNING !!! kIdxLast not correct, also map indexes not corrected
    auto [updated, val] = _node_this->Set(key, _node_insert);
    return {updated, key, updated ? XValueRT() : val /*XValue(_node_insert)*/};
}

INode::SPtr xnode::NodeGetByKey(INode* const                         _node_this_p,
                                const XKey&                          _key,
                                const std::optional<INode::NodeType> _node_type,
                                const bool                           _convert_to_type)
{
    if (!_node_this_p)
        return nullptr;

    INode::SPtr node_new_sp;
    auto        xval = _node_this_p->At(_key);
    while (true) {
        auto node_p = xval.QueryPtr<INode>();
        if (node_p && node_p->Type() == _node_type.value_or(node_p->Type()))
            return node_p;

        if (!xval.IsEmpty() && !_convert_to_type)
            return nullptr;

        if (!node_new_sp && _node_type.has_value())
            node_new_sp = xnode::Create(_node_type.value());
        if (!node_new_sp)
            return nullptr;

        auto [success, current] = _node_this_p->CompareExchange(_key, xval, node_new_sp);
        if (success)
            return node_new_sp;

        if (current.Type() != XValue::kObject) {
            assert(true);
        }

        xval = current;
    }

    return nullptr;
}

INode::SPtrC xnode::NodeConstGetByKey(const INode* const _node_this_p, const XKey& _key)
{
    if (!_node_this_p)
        return nullptr;

    return _node_this_p->At(_key).QueryPtrC<INode>();
}

} // namespace xsdk