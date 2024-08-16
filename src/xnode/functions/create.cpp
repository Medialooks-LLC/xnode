#include "../xkey_type_match.h"
#include "xnode_factory.h"
#include "xnode_functions.h"

namespace xsdk {

INode::SPtr xnode::Create(INode::NodeType _type, std::string_view _name /*= {}*/, uint64_t _uid /*= 0*/)
{
    return XNodeFactoryGet()->NodeCreate(_type, _name, _uid);
}

INode::SPtr xnode::CreateArray(std::vector<XValue>&& _values,
                                   std::string_view      _name /*= {}*/,
                                   uint64_t              _uid /*= 0*/)
{
    auto node_p = xnode::Create(INode::NodeType::Array, _name, _uid);
    if (node_p)
        node_p->BulkInsert(XKey(), std::move(_values));

    return node_p;
}

INode::SPtr xnode::CreateMap(std::vector<std::pair<XKey, XValue>>&& _values,
                                 std::string_view                       _name /*= {}*/,
                                 uint64_t                               _uid /*= 0*/)
{
    auto node_p = xnode::Create(INode::NodeType::Map, _name, _uid);
    if (node_p)
        node_p->BulkSet(std::move(_values));

    return node_p;
}

INode::SPtr xnode::CreateComplex(std::vector<std::pair<XPath, XValue>>&& _values,
                                     std::string_view                        _name /*= {}*/,
                                     uint64_t                                _uid /*= 0*/)
{
    std::optional<INode::NodeType> type;
    if (!_values.empty())
        type = XKeyToNodeType::Match(_values[0].first.front());

    auto node_p = xnode::Create(type.value_or(INode::NodeType::Map), _name, _uid);

    for (auto&& [key, value] : _values)
        xnode::Set(node_p, std::move(key), std::move(value));

    return node_p;
}

} // namespace xsdk