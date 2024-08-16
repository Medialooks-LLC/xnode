#include <iostream>

#include "xml_creator.h"

#include "xnode_interfaces.h"

namespace xsdk::impl {


XmlDocCreator::XmlDocCreator(INode::SPtrC _node, std::string_view _attribute_prefix, std::string_view _value_name)
    : root_node_(_node),
      attribute_prefix_(_attribute_prefix),
      value_name_(_value_name),
      pref_len_(_attribute_prefix.size())
{
    tr_ = std::make_unique<impl::Transcoder>();
}

std::string_view XmlDocCreator::CutPrefix_(const XKey& key) const
{
    return key.StringGet().value_or("").substr(pref_len_, key.StringGet().value_or("").size());
}

XC::DOMDocument* XmlDocCreator::GetDocument()
{
    if (!root_node_)
        return nullptr;

    auto impl      = XC::DOMImplementationRegistry::getDOMImplementation(tr_->ToXmlChars("Core")->data());
    auto root_name = root_node_->IsName("") ? GetNameForUnnamedNode_() : root_node_->NameGet();
    if (impl) {
        doc_.reset(impl->createDocument(0, tr_->ToXmlChars(root_name.c_str())->data(), 0));
        auto root = doc_->getDocumentElement();
        for (const auto& [key, xval] : root_node_->BulkGetAll()) {
            assert(!key.StringGet().value_or("").empty());
            auto child_node = xval.QueryPtrC<INode>();
            if (child_node) {
                AddNode_(child_node, root);
            }
            else {
                auto key_value = key.StringGet().value_or("").data();
                if (key_value == value_name_ || key_value == root_name) {
                    auto value = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
                    root->appendChild(value);
                }
                else {
                    if (key.StringGet()->rfind(attribute_prefix_, 0) == 0){ // starts with attribute_prefix_
                        root->setAttribute(tr_->ToXmlChars(CutPrefix_(key).data())->data(),
                                           tr_->ToXmlChars(xval.String().data())->data());
                    } else {
                        auto element = doc_->createElement(
                            tr_->ToXmlChars(key.StringGet().value_or(GetNameForUnnamedNode_()).data())->data());
                        auto value = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
                        element->appendChild(value);
                        root->appendChild(element);
                    }
                }
            }
        }
        return doc_.get();
    }
    return nullptr;
}

void XmlDocCreator::AddNode_(const INode::SPtrC& _node, XC::DOMElement* _parent, bool _wrapped)
{
    if (_node->Type() == INode::NodeType::Map) {
        AddMapNode_(_node, _parent, _wrapped);
    }
    else {
        assert(_node->Type() == INode::NodeType::Array);
        auto element_name = _node->IsName("") ? GetNameForUnnamedNode_() : _node->NameGet();
        bool has_text_value     = false;
        if (!_node->IsName("")){
            auto has_same_name = _node->ParentGet()->IsName(element_name);
            auto has_value_name = _node->IsName(value_name_);
            has_text_value      = has_same_name || has_value_name;
        }
        if (has_text_value) {
            AddArrayNodeAsText_(_node, _parent);
        } else {
            AddArrayNode_(_node, _parent);
        }
    }
}

void XmlDocCreator::AddMapNode_(const INode::SPtrC& _node, XC::DOMElement* _parent, bool _wrapped)
{
    XC::DOMElement* element;
    std::string element_name;
    if (_node->ParentGet()->Type() == INode::NodeType::Map || _wrapped) {
        element = doc_->createElement(tr_->ToXmlChars(_node->NameGet().data())->data());
    }
    else {
        assert(_node->ParentGet()->Type() == INode::NodeType::Array);
        if (_node->ParentGet()->IsName("")) {
            element      = doc_->createElement(_parent->getTagName());
        }else{
            element = doc_->createElement(tr_->ToXmlChars(_node->ParentGet()->NameGet().data())->data());
        }
    }
    _parent->appendChild(element);
    for (const auto& [key, xval] : _node->BulkGetAll()) {
        assert(!key.StringGet().value_or("").empty());
        auto child_node = xval.QueryPtrC<INode>();
        if (child_node) {
            AddNode_(child_node, element);
        }
        else {
            auto key_value = key.StringGet().value_or("").data();
            auto tag_name  = tr_->ToString(element->getTagName());
            if (key_value == value_name_ || key_value == tag_name) {
                auto value = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
                element->appendChild(value);
            }
            else {
                if (key.StringGet()->rfind(attribute_prefix_, 0) == 0){// starts with attribute_prefix_
                    element->setAttribute(tr_->ToXmlChars(CutPrefix_(key).data())->data(),
                                       tr_->ToXmlChars(xval.String().data())->data());
                }
                else {
                    auto element_wrap = doc_->createElement(
                        tr_->ToXmlChars(key.StringGet().value_or(GetNameForUnnamedNode_()).data())->data());
                    auto value = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
                    element_wrap->appendChild(value);
                    element->appendChild(element_wrap);
                }
            }
        }
    }
}

void XmlDocCreator::AddArrayNode_(const INode::SPtrC& _node, XC::DOMElement* _parent)
{
    auto element_name = _node->IsName("") ? GetNameForUnnamedNode_() : _node->NameGet();
    for (const auto& [key, xval] : _node->BulkGetAll()) {
        auto child_node = xval.QueryPtrC<INode>();
        if (child_node) {
            if (child_node->IsName("")) {
                AddNode_(child_node, _parent);
            }
            else {
                auto element = doc_->createElement(tr_->ToXmlChars(element_name.data())->data());
                AddNode_(child_node, element, true);
                _parent->appendChild(element);
            }
        }
        else {
            auto element = doc_->createElement(tr_->ToXmlChars(element_name.data())->data());
            auto value   = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
            element->appendChild(value);
            _parent->appendChild(element);
        }
    }
}

void XmlDocCreator::AddArrayNodeAsText_(const INode::SPtrC& _node, XC::DOMElement* _parent) {
    auto element = _parent;
    for (const auto& [key, xval] : _node->BulkGetAll()) {
        auto child_node = xval.QueryPtrC<INode>();
        if (child_node) {
            AddNode_(child_node, element, true);
        }
        else {
            auto value   = doc_->createTextNode(tr_->ToXmlChars(xval.String().data())->data());
            element->appendChild(value);
        }
    }
}

std::string XmlDocCreator::GetNameForUnnamedNode_() { return "noname"; }

} // namespace xsdk::impl