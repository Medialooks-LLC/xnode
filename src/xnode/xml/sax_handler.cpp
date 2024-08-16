#include "sax_handler.h"

#include "transcoder.h"
#include "xnode_interfaces.h"
#include "xnode_functions.h"

#include <iostream>
#include <iterator>

#include "xercesc/sax2/Attributes.hpp"
#include "xercesc/sax2/DefaultHandler.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

static bool is_formating(std::string& str)
{
    if (std::find_if(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); }) == str.end())
        return true;
    return false;
}

XNodeSaxHandler::XNodeSaxHandler(uint64_t         _uid,
                                 std::string_view _name,
                                 std::string_view _attribute_prefix,
                                 std::string_view _value_name)
    : XC::DefaultHandler(),
      root_uid_(_uid),
      root_name_(_name),
      attr_prefix_(_attribute_prefix),
      value_name_(_value_name)
{
    coder_ = std::make_unique<xsdk::impl::Transcoder>();
};

void XNodeSaxHandler::startElement(const XMLCh* const    _uri,
                                   const XMLCh* const    _localname,
                                   const XMLCh* const    _qname,
                                   const XC::Attributes& _attrs)
{
    has_more_chars_ = false;
    PutNode_(coder_->ToString(_localname));
    for (XMLSize_t i = 0; i < _attrs.getLength(); i++) {
        key_ = XKey(std::string(attr_prefix_).append(coder_->ToString(_attrs.getLocalName(i))));
        PutValue_(coder_->ToString(_attrs.getValue(i)));
    }
}

void XNodeSaxHandler::characters(const XMLCh* const _chars, const XMLSize_t _length)
{
    key_ = value_name_;
    auto val = coder_->ToString(_chars);
    if (is_formating(val))
        return;
    PutValue_(val);
    has_more_chars_ = true;
}

void XNodeSaxHandler::endElement(const XMLCh* const _uri, const XMLCh* const _localname, const XMLCh* const _qname)
{
    has_more_chars_ = false;
    EndNode_(coder_->ToString(_localname));
}

void XNodeSaxHandler::PutValue_(XValue&& _val, bool _is_node)
{
    if (nodes_stack_.empty()) {
        assert(!root);
        root = std::move(_val);
        return;
    }

    auto is_text = key_.StringGet() == value_name_;
    if (is_text) {
        if (has_more_chars_) {
            auto last_key = GetLastAndNewKeys_(nodes_stack_.back()).first;
            nodes_stack_.back()->Append(last_key, _val.StringView());
            return;
        }
        else {
            key_ = GetLastAndNewKeys_(nodes_stack_.back()).second;
        }
    }

    if (_is_node || is_text) {
        StoreNodesOrder_();
    }

    auto success = nodes_stack_.back()->Insert(key_, std::move(_val)).succeeded;
    assert(success);
}

std::pair<XKey, XKey> XNodeSaxHandler::GetLastAndNewKeys_(const INode::SPtr& _node) const
{
    auto new_key  = XKey(value_name_);
    auto last_key = new_key;
    auto counter  = 0;
    while (_node->At(new_key)) {
        last_key = new_key;
        new_key  = XKey(std::to_string(counter).append(value_name_));
        counter++;
    }
    return {last_key, new_key};
}

void XNodeSaxHandler::StoreNodesOrder_()
{
    auto key_str = key_.StringGet() ? key_.StringGet().value().data() : (assert(false), "");
    if (nodes_order_.count(nodes_stack_.back()) != 0) {
        nodes_order_.at(nodes_stack_.back()).push_back(key_str);
    }
    else {
        nodes_order_[nodes_stack_.back()] = {key_str};
    }
}

void XNodeSaxHandler::PutNode_(std::string_view _node_name)
{
    auto key    = root ? root_name_.empty()
                       ? _node_name : root_name_
                                    : _node_name;

    key_        = key;
    auto node_p = xnode::Create(INode::NodeType::Map, key, nodes_stack_.empty() ? root_uid_ : 0);

    if (nodes_stack_.empty()) {
        PutValue_(node_p, true);
        nodes_stack_.push_back(node_p);
        return;
    }

    if (nodes_stack_.back()->Type() == INode::NodeType::Map) {
        auto does_parent_already_have_key = nodes_stack_.back()->At(_node_name);
        if (does_parent_already_have_key) {
            StartArray_(_node_name);
        }
        PutValue_(node_p, true);
    }
    else {
        auto is_array_element = nodes_stack_.back()->IsName(_node_name);
        if (is_array_element) {
            PutValue_(node_p, true);
        }
        else {
            auto node_wrapper = WrapToNode_(node_p);
            PutValue_(node_wrapper, true);
        }
    }

    nodes_stack_.push_back(node_p);
}

void XNodeSaxHandler::EndNode_(std::string_view _node_name)
{
    if (nodes_stack_.empty()) {
        assert(false);
        return;
    }

    auto last_popped_node = nodes_stack_.back();
    nodes_stack_.pop_back();

    if (!last_popped_node->IsName(_node_name)) {
        last_popped_node = EndArray_(_node_name);
    }

    if (HasNestedNodes_(last_popped_node)) {
        ConvertToArray_(last_popped_node);
    }

    PropagateNodeName_(last_popped_node);
}

void XNodeSaxHandler::StartArray_(std::string_view _node_name)
{
    auto node_arr = xnode::Create(INode::NodeType::Array,
                                  nodes_stack_.empty() ? (root_name_.empty() ? _node_name : root_name_) : _node_name,
                                  nodes_stack_.empty() ? root_uid_ : 0);
    auto parent   = nodes_stack_.back();
    if (nodes_order_.find(parent) != nodes_order_.end()) {
        for (auto name : nodes_order_.at(parent)) {
            auto child = parent->At(name);
            parent->Erase(name);
            if (_node_name.compare(name) == 0) {
                auto success = node_arr->Insert(kIdxEnd, std::move(child)).succeeded;
                assert(success);
            }
            else {
                auto child_wrapper = WrapToNode_(name, child);
                auto success       = node_arr->Insert(kIdxEnd, std::move(child_wrapper)).succeeded;
                assert(success);
            }
        }
    }
    else {
        assert(false);
    }
    auto [success, _] = parent->Set(_node_name, node_arr);
    assert(success);
    nodes_stack_.push_back(node_arr);
    array_parent_nodes_.push_back(node_arr);
    deep_++;
}

INode::SPtr XNodeSaxHandler::EndArray_(std::string_view _node_name)
{
    auto last_popped_node = nodes_stack_.back();
    nodes_stack_.pop_back();
    deep_--;
    assert(last_popped_node->IsName(_node_name));
    assert(deep_ >= 0);
    CollapseArrayNodes_(array_parent_nodes_.back());
    nodes_order_.erase(array_parent_nodes_.back());
    array_parent_nodes_.pop_back();
    return last_popped_node;
}

void XNodeSaxHandler::CollapseArrayNodes_(const INode::SPtr& _node)
{
    auto parent_name = _node->NameGet();
    auto to_update   = std::vector<XKey> {};
    _node->ForEach([&v_name = value_name_, &attr_prefix = attr_prefix_, &parent_name, &to_update](const XKey& _key,
                                                                                                  XValueRT&   _value) {
        auto child = _value.QueryPtr<INode>();
        if (child && child->Size() == 1) {
            auto elements = child->BulkGetAll(); // how to get name of element
            auto key      = elements.back().first;
            if (key.StringGet().value().compare(0, attr_prefix.size(), attr_prefix) != 0) {
                to_update.push_back(_key);
            }
        }
        return OnEachRes::Next;
    });
    for (auto key : to_update) {
        auto child   = _node->At(key).QueryPtr<INode>();
        auto success = _node->Set(key, child->At(kIdxBegin)).first;
        assert(success);
    }
}

bool XNodeSaxHandler::HasNestedNodes_(const INode::SPtr& _node)
{
    bool has_nested_nodes = false;
    if (_node->At(value_name_)) {
        _node->ForEach([&has_nested_nodes](const XKey& key, XValueRT& value) {
            if (value.IsObject()) {
                has_nested_nodes = true;
                return OnEachRes::Stop;
            }
            return OnEachRes::Next;
        });
    }
    return has_nested_nodes;
}

void XNodeSaxHandler::ConvertToArray_(const INode::SPtr& _node)
{
    auto node_arr = xnode::Create(INode::NodeType::Array, value_name_, 0);
    for (std::string_view element_name : nodes_order_.at(_node)) {
        auto child           = _node->At(element_name).QueryPtr<INode>();
        auto need_wrap_array = child && child->Type() == INode::NodeType::Array;
        if (need_wrap_array) {
            auto child_wrapper = WrapToNode_(child);
            auto success       = node_arr->Insert(kIdxEnd, std::move(child_wrapper)).succeeded;
            assert(success);
        }
        else {
            auto success = node_arr->Insert(kIdxEnd, std::move(_node->At(element_name))).succeeded;
            assert(success);
        }
        _node->Erase(element_name);
    }
    auto is_empty_node_with_map_parent = _node->Size() == 0 && _node->ParentGet() &&
                                         _node->ParentGet()->Type() == INode::NodeType::Map;
    if (is_empty_node_with_map_parent) {
        auto parent  = _node->ParentDetach();
        auto arr_wrapper = WrapToNode_(node_arr);
        auto success = parent->Set(_node->NameGet(), arr_wrapper).first;
        assert(success);
    }
    else {
        _node->Set(value_name_, node_arr);
    }
}

void XNodeSaxHandler::PropagateNodeName_(const INode::SPtr& _node)
{
    if (!_node->IsName("")) {
        if (_node->Size() == 1 && _node->At(value_name_)) {
            _node->KeyChange(value_name_, _node->NameGet());
        }
    }
}

inline INode::SPtr XNodeSaxHandler::WrapToNode_(const INode::SPtr& _node)
{
    auto wrapper = xnode::Create(INode::NodeType::Map, _node->NameGet());
    auto success = wrapper->Insert(_node->NameGet(), _node).succeeded;
    assert(success);
    return wrapper;
}

inline INode::SPtr XNodeSaxHandler::WrapToNode_(std::string_view _name, XValueRT _value)
{
    auto wrapper = xnode::Create(INode::NodeType::Map, _name);
    auto success = wrapper->Insert(_name, std::move(_value)).succeeded;
    assert(success);
    return wrapper;
}

} // namespace xsdk::impl