#pragma once
#include <iostream>
#include "transcoder.h"
#include "xml_helpers.h"
#include "xnode_interfaces.h"

#include "xercesc/dom/DOM.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

class XmlDocCreator {
public:
    XmlDocCreator(INode::SPtrC     _node,
                  std::string_view _attribute_prefix,
                  std::string_view _value_name);

    XC::DOMDocument* GetDocument();

private:
    INode::SPtrC                      root_node_;
    const std::string                 attribute_prefix_;
    const std::string                 value_name_;
    const size_t                      pref_len_;
    XmlUniquePtr<XC::DOMDocument>     doc_;
    std::unique_ptr<impl::Transcoder> tr_;

    void AddNode_(const INode::SPtrC& _node, XC::DOMElement* _parent, bool _wrapped = false);

    void AddMapNode_(const INode::SPtrC& _node, XC::DOMElement* _parent, bool _wrapped = false);
    void AddArrayNode_(const INode::SPtrC& _node, XC::DOMElement* _parent);
    void AddArrayNodeAsText_(const INode::SPtrC& _node, XC::DOMElement* _parent);

    std::string_view CutPrefix_(const XKey& key) const;

    static std::string GetNameForUnnamedNode_();
};

} // namespace xsdk::impl