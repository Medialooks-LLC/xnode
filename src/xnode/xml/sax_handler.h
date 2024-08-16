#include "xnode_xml.h"

#include "transcoder.h"
#include "xnode_interfaces.h"

#include <iostream>

#include "xercesc/sax2/DefaultHandler.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

class XNodeSaxHandler final: public XC::DefaultHandler {
public:
    XValue root;

private:
    const uint64_t    root_uid_;
    const std::string root_name_;
    const std::string attr_prefix_;
    const std::string value_name_;

    XKey                     key_;
    std::vector<INode::SPtr> nodes_stack_;
    std::vector<INode::SPtr> array_parent_nodes_;
    uint64_t                 deep_ {0};
    bool                     has_more_chars_ {false};

    std::map<INode::SPtr, std::vector<std::string>> nodes_order_;
    std::unique_ptr<xsdk::impl::Transcoder>         coder_;
    const size_t                                    max_utf8_symbol_size_ = 4;

public:
    XNodeSaxHandler(uint64_t         _uid              = 0,
                    std::string_view _name             = {},
                    std::string_view _attribute_prefix = kXMLAttributePrefix,
                    std::string_view _value_name       = kXMLValueName);

    void startElement(const XMLCh* const    _uri,
                      const XMLCh* const    _localname,
                      const XMLCh* const    _qname,
                      const XC::Attributes& _attrs) override;

    void characters(const XMLCh* const _chars, const XMLSize_t _length) override;

    void endElement(const XMLCh* const _uri, const XMLCh* const _localname, const XMLCh* const _qname) override;

private:
    void PutValue_(XValue&& _val, bool _is_node = false);
    void PutNode_(std::string_view _node_name);
    void EndNode_(std::string_view _node_name);

    std::pair<XKey, XKey> GetLastAndNewKeys_(const INode::SPtr& _node) const;
    void                  StoreNodesOrder_();

    void        StartArray_(std::string_view _node_name);
    INode::SPtr EndArray_(std::string_view _node_name);
    void        CollapseArrayNodes_(const INode::SPtr& _node);

    bool HasNestedNodes_(const INode::SPtr& _node);
    void ConvertToArray_(const INode::SPtr& _node);

    void PropagateNodeName_(const INode::SPtr& _node);

    INode::SPtr WrapToNode_(const INode::SPtr& _node);
    INode::SPtr WrapToNode_(std::string_view _name, XValueRT _value);
};

} // namespace xsdk::impl