#include "xnode_json.h"
#include "xnode_factory.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace xsdk {

struct XNodeJsonHandler: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, XNodeJsonHandler> {

    XValue root_;

private:
    XKey                     key_;
    std::vector<INode::SPtr> nodes_;

    const uint64_t         root_uid_;
    const std::string_view root_name_;

public:
    XNodeJsonHandler(const uint64_t _uid, const std::string_view _name) : root_uid_(_uid), root_name_(_name) {}

    bool Null() { return PutValue_(XValue(nullptr)); }
    bool Bool(bool _val) { return PutValue_(XValue(_val)); }
    bool Int(int _val) { return PutValue_(XValue(_val)); }
    bool Uint(unsigned _val) { return PutValue_(XValue(_val)); }
    bool Int64(int64_t _val) { return PutValue_(XValue(_val)); }
    bool Uint64(uint64_t _val) { return PutValue_(XValue(_val)); }
    bool Double(double _val) { return PutValue_(XValue(_val)); }
    bool String(const char* _str, rapidjson::SizeType _length, bool _copy)
    {
        return PutValue_(std::string_view(_str, _length));
    }
    bool Key(const char* _str, rapidjson::SizeType _length, bool _copy)
    {
        key_ = std::string(_str, _length);
        return true;
    }

    bool StartObject() { return PutNode_(INode::NodeType::Map); }

    bool EndObject(rapidjson::SizeType _member_count)
    {
        assert(!nodes_.empty() && nodes_.back() && nodes_.back()->Size() == _member_count &&
               nodes_.back()->Type() == INode::NodeType::Map);

        return EndNode_();
    }

    bool StartArray() { return PutNode_(INode::NodeType::Array); }

    bool EndArray(rapidjson::SizeType _element_count)
    {
        assert(!nodes_.empty() && nodes_.back() && nodes_.back()->Size() == _element_count &&
               nodes_.back()->Type() == INode::NodeType::Array);

        return EndNode_();
    }

private:
    bool PutValue_(XValue&& _val)
    {
        if (nodes_.empty()) {
            assert(!key_ && !root_);
            root_ = std::move(_val);
            return true;
        }

        auto [success, insert_at, prev] = nodes_.back()->Insert(std::exchange(key_, kIdxEnd), std::move(_val));
        assert(success);
        return success;
    }

    bool PutNode_(const INode::NodeType _node_type)
    {
        auto node_p = XNodeFactoryGet()->NodeCreate(_node_type,
                                                    nodes_.empty() ? root_name_ : std::string_view(),
                                                    nodes_.empty() ? root_uid_ : 0);
        PutValue_(node_p);
        nodes_.push_back(node_p);
        return true;
    }

    bool EndNode_()
    {
        if (nodes_.empty())
            return false;

        nodes_.pop_back();
        return true;
    }
};

std::pair<INode::SPtr, size_t> xnode::FromJson(const std::string_view _json,
                                               const std::string_view _name,
                                               const uint64_t         _uid)
{
    if (_json.empty())
        return {nullptr, -1};

    XNodeJsonHandler handler(_uid, _name);

    rapidjson::Reader       reader;
    rapidjson::StringStream ssInput(_json.data());
    auto                    res       = reader.Parse(ssInput, handler);
    size_t                  error_pos = 0;
    if (res.IsError())
        error_pos = res.Offset() != 0 ? res.Offset() : -1;

    return {handler.root_.QueryPtr<INode>(), error_pos};
}

// Serialization to json
template <class TWriter>
void WriteXValue(TWriter&& _writer, const XValueRT& _value_at, const xnode::JsonFormat _json_format)
{
    switch (_value_at.Type()) {
        case XValue::kEmpty: // 2Think !!!
        case XValue::kNull:
            _writer.Null();
            break;
        case XValue::kBool:
            _writer.Bool(_value_at.Bool());
            break;
        case XValue::kInt64:
            _writer.Int64(_value_at.Int64());
            break;
        case XValue::kUint64:
            _writer.Int64(_value_at.Uint64());
            break;
        case XValue::kDouble:
            _writer.Double(_value_at.Double());
            break;
        case XValue::kString:
            _writer.String(_value_at.StringView().data(),
                           static_cast<rapidjson::SizeType>(_value_at.StringView().size()));
            break;
        case XValue::kObject:
        case XValue::kConstObject:
            WriteXNode(_value_at.QueryPtrC<INode>(), _writer, _json_format);
            break;

        default:
            assert(!"WriteXValue - unknown type");
            break;
    }
}

template <class TWriter>
void WriteXNode(const INode::SPtrC& _node_sp, TWriter&& _writer, const xnode::JsonFormat _json_format)
{
    if (_node_sp->Type() == INode::NodeType::Map) {
        _writer.StartObject();
        auto test_vec = _node_sp->BulkGetAll();
        for (const auto& [key, xval] : _node_sp->BulkGetAll()) {
            assert(!key.StringGet().value_or("").empty());
            auto key_str = key.StringGet();
            _writer.Key(key_str->data(), static_cast<rapidjson::SizeType>(key_str->size()));
            WriteXValue(_writer, xval, _json_format);
        }
        _writer.EndObject();
    }
    else {
        assert(_node_sp->Type() == INode::NodeType::Array);
        _writer.StartArray();
        for (const auto& [key_idx, xval] : _node_sp->BulkGetAll())
            WriteXValue(_writer, xval, _json_format);
        _writer.EndArray();
    }
}

std::string xnode::ToJson(const INode::SPtrC&     _node_this,
                          const xnode::OnCopyPF&  _pf_on_item,
                          const xnode::JsonFormat _json_format,
                          const size_t            _indent_char_count,
                          const char              _indent_char)
{
    if (!_node_this)
        return {};

    rapidjson::StringBuffer s;
    if (JsonFormat::kOneLine == _json_format) {
        WriteXNode(_node_this, rapidjson::Writer<rapidjson::StringBuffer>(s), _json_format);
    }
    else {
        auto writer = rapidjson::PrettyWriter<rapidjson::StringBuffer>(s);
        writer.SetFormatOptions(JsonFormat::kOneLineArrays == _json_format ? rapidjson::kFormatSingleLineArray :
                                                                             rapidjson::kFormatDefault);
        writer.SetIndent(_indent_char, (uint32_t)_indent_char_count);
        WriteXNode(_node_this, std::move(writer), _json_format);
    }

    return {s.GetString(), s.GetSize()};
}

} // namespace xsdk
