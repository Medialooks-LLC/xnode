#include "xnode_json.h"
#include "xnode_factory.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace xsdk {

struct XNodeJsonHandler: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, XNodeJsonHandler> {

    XValue root;

private:
    XKey                     key;
    std::vector<INode::SPtr> nodes;

    uint64_t         root_uid;
    std::string_view root_name;

public:
    XNodeJsonHandler(uint64_t _uid = 0, std::string_view _name = {}) : root_uid(_uid), root_name(_name) {}

    bool Null() { return _put_value(XValue(nullptr)); }
    bool Bool(bool b) { return _put_value(XValue(b)); }
    bool Int(int i) { return _put_value(XValue(i)); }
    bool Uint(unsigned u) { return _put_value(XValue(u)); }
    bool Int64(int64_t i) { return _put_value(XValue(i)); }
    bool Uint64(uint64_t u) { return _put_value(XValue(u)); }
    bool Double(double d) { return _put_value(XValue(d)); }
    bool String(const char* str, rapidjson::SizeType length, bool copy)
    {
        return _put_value(std::string_view(str, length));
    }
    bool Key(const char* str, rapidjson::SizeType length, bool copy)
    {
        key = std::string(str, length);
        return true;
    }

    bool StartObject() { return _put_node(INode::NodeType::Map); }

    bool EndObject(rapidjson::SizeType memberCount)
    {
        assert(!nodes.empty() && nodes.back() && nodes.back()->Size() == memberCount &&
               nodes.back()->Type() == INode::NodeType::Map);

        return _end_node();
    }

    bool StartArray() { return _put_node(INode::NodeType::Array); }

    bool EndArray(rapidjson::SizeType elementCount)
    {
        assert(!nodes.empty() && nodes.back() && nodes.back()->Size() == elementCount &&
               nodes.back()->Type() == INode::NodeType::Array);

        return _end_node();
    }

private:
    bool _put_value(XValue&& _val)
    {
        if (nodes.empty()) {
            assert(!key && !root);
            root = std::move(_val);
            return true;
        }

        auto [success, insert_at, prev] = nodes.back()->Insert(std::exchange(key, kIdxEnd), std::move(_val));
        assert(success);
        return success;
    }

    bool _put_node(INode::NodeType _node_type)
    {
        auto node_p = XNodeFactoryGet()->NodeCreate(_node_type,
                                                    nodes.empty() ? root_name : std::string_view(),
                                                    nodes.empty() ? root_uid : 0);
        _put_value(node_p);
        nodes.push_back(node_p);
        return true;
    }

    bool _end_node()
    {
        if (nodes.empty())
            return false;

        nodes.pop_back();
        return true;
    }
};

std::pair<INode::SPtr, size_t> xnode::FromJson(std::string_view _json, uint64_t _uid, std::string_view _name)
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

    return {handler.root.QueryPtr<INode>(), error_pos};
}

// Serialization to json
template <class TWriter>
void WriteXValue(TWriter&& writer, const XValueRT& ValueAt_, xnode::JsonFormat _json_format)
{
    switch (ValueAt_.Type()) {
        case XValue::kEmpty: // 2Think !!!
        case XValue::kNull:
            writer.Null();
            break;
        case XValue::kBool:
            writer.Bool(ValueAt_.Bool());
            break;
        case XValue::kInt64:
            writer.Int64(ValueAt_.Int64());
            break;
        case XValue::kUint64:
            writer.Int64(ValueAt_.Uint64());
            break;
        case XValue::kDouble:
            writer.Double(ValueAt_.Double());
            break;
        case XValue::kString:
            writer.String(ValueAt_.StringView().data(), static_cast<rapidjson::SizeType>(ValueAt_.StringView().size()));
            break;
        case XValue::kObject:
        case XValue::kConstObject:
            WriteXNode(ValueAt_.QueryPtrC<INode>(), writer, _json_format);
            break;

        default:
            assert(!"write_xvalue - unknown type");
            break;
    }
}

template <class TWriter>
void WriteXNode(const INode::SPtrC& _node_sp, TWriter&& writer, xnode::JsonFormat _json_format)
{
    if (_node_sp->Type() == INode::NodeType::Map) {
        writer.StartObject();
        auto test_vec = _node_sp->BulkGetAll();
        for (const auto& [key, xval] : _node_sp->BulkGetAll()) {
            assert(!key.StringGet().value_or("").empty());
            writer.Key(key.StringGet().value().data(),
                       static_cast<rapidjson::SizeType>(key.StringGet().value().size()));
            WriteXValue(writer, xval, _json_format);
        }
        writer.EndObject();
    }
    else {
        assert(_node_sp->Type() == INode::NodeType::Array);
        writer.StartArray();
        for (const auto& [key_idx, xval] : _node_sp->BulkGetAll())
            WriteXValue(writer, xval, _json_format);
        writer.EndArray();
    }
}

std::string xnode::ToJson(const INode::SPtrC& _node_this,
                          xnode::OnCopyPF&    _pf_on_item,
                          xnode::JsonFormat   _json_format,
                          size_t              _indent_char_count,
                          char                _indent_char)
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
