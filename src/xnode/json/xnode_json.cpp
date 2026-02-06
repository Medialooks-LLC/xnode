#include "xnode_json.h"
#include "xnode_factory.h"

#define RAPIDJSON_HAS_CXX17 1
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace xsdk {

namespace impl {

    bool IsCustomObject(const XValue& _val)
    {
        if (_val.Type() != XValue::kObject && _val.Type() != XValue::kConstObject)
            return false;

        if (_val.QueryPtrC<INode>())
            return false;

        return true;
    }

    // Forward declaration
    template <class TWriter>
    void WriteXNode(TWriter* const                    _writer_p,
                    const INode*                      _node_p,
                    const xnode::JsonFormat           _json_format,
                    const xnode::OnCustomSerializePf& _pf_on_custom);

    // Serialization to json
    template <class TWriter>
    void WriteXValue(TWriter* const                    _writer_p,
                     XValueRT&&                        _value,
                     const xnode::JsonFormat           _json_format,
                     const xnode::OnCustomSerializePf& _pf_on_custom,
                     const XKey&                       _key_for_cb)
    {
        assert(_writer_p);

        bool repeat_write = true;
        while (std::exchange(repeat_write, false)) {

            switch (_value.Type()) {
                case XValue::kEmpty: // 2Think !!!
                case XValue::kNull:
                    _writer_p->Null();
                    break;
                case XValue::kBool:
                    _writer_p->Bool(_value.Bool());
                    break;
                case XValue::kInt64:
                    _writer_p->Int64(_value.Int64());
                    break;
                case XValue::kUint64:
                    _writer_p->Int64(_value.Uint64());
                    break;
                case XValue::kDouble:
                    _writer_p->Double(_value.Double());
                    break;
                case XValue::kString:
                    _writer_p->String(_value.StringView().data(),
                                      static_cast<rapidjson::SizeType>(_value.StringView().size()));
                    break;
                case XValue::kObject:
                case XValue::kConstObject: {
                    auto node_sp = _value.QueryPtrC<INode>();
                    if (node_sp) {
                        WriteXNode(_writer_p, node_sp.get(), _json_format, _pf_on_custom);
                    }
                    else {
                        auto obj_sp = _value.ObjectPtrC();
                        assert(obj_sp);
                        auto xvalue = _pf_on_custom ? _pf_on_custom(_key_for_cb, obj_sp.get()) : XValue {};
                        assert(!IsCustomObject(xvalue)); // For prevent infinite cicle
                        if (xvalue && !IsCustomObject(xvalue)) {
                            // Repeat writing for updated value
                            _value       = std::move(xvalue);
                            repeat_write = true;
                        }
                        else {
                            // Custom object stub
                            std::string stub;
                            const auto  buf_p = _value.QueryPtrC<xbase::IBuffer>();
                            if (buf_p) {
                                stub = "[IBuffer(" + std::to_string(buf_p->ObjectUid()) +
                                       "):" + std::to_string(buf_p->BufferData().size()) + "]";
                            }
                            else if (obj_sp) {
                                stub = "[custom object:" + std::to_string(obj_sp->ObjectUid()) + "]";
                            }
                            else {
                                stub = "[###Err### - null custom object]";
                            }
                            _writer_p->String(stub.c_str());
                        }
                    }
                    break;
                }

                default:
                    assert(!"WriteXValue - unknown type");
                    break;
            }
        }
    }

    template <class TWriter>
    void WriteXNode(TWriter* const                    _writer_p,
                    const INode*                      _node_p,
                    const xnode::JsonFormat           _json_format,
                    const xnode::OnCustomSerializePf& _pf_on_custom)
    {
        assert(_node_p);

        if (_node_p->Type() == INode::NodeType::Map) {
            _writer_p->StartObject();
            auto test_vec = _node_p->BulkGetAll();
            for (auto& [key, xval] : _node_p->BulkGetAll()) {
                assert(!key.StringGet().value_or("").empty());
                auto key_str = key.StringGet();
                _writer_p->Key(key_str->data(), static_cast<rapidjson::SizeType>(key_str->size()));
                WriteXValue(_writer_p, std::move(xval), _json_format, _pf_on_custom, key);
            }
            _writer_p->EndObject();
        }
        else {
            assert(_node_p->Type() == INode::NodeType::Array);
            _writer_p->StartArray();
            for (auto& [key_idx, xval] : _node_p->BulkGetAll())
                WriteXValue(_writer_p, std::move(xval), _json_format, _pf_on_custom, key_idx);

            _writer_p->EndArray();
        }
    }

    struct XNodeJsonHandler: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, XNodeJsonHandler> {

        XValue root_;

    private:
        XKey                                      recent_key_;
        std::vector<std::pair<XKey, INode::SPtr>> nodes_w_keys_;

        const xnode::OnCustomDeserializePf     on_custom_deserialize_;
        const std::optional<XValue::ValueType> callback_types_mask_;
        const std::optional<uint64_t>          root_uid_;
        // const std::string_view                 root_name_;

    public:
        XNodeJsonHandler(const xnode::OnCustomDeserializePf&    _on_custom_deserialize,
                         const std::optional<XValue::ValueType> _callback_types_mask,
                         const std::optional<xbase::Uid>        _node_uid,
                         const std::string_view                 _name)
            : on_custom_deserialize_(_on_custom_deserialize),
              callback_types_mask_(_callback_types_mask),
              root_uid_(_node_uid),
              recent_key_(_name)
        {
        }

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
            recent_key_ = std::string(_str, _length);
            return true;
        }

        bool StartObject() { return PutNode_(INode::NodeType::Map); }

        bool EndObject(rapidjson::SizeType _member_count)
        {
            assert(!nodes_w_keys_.empty() && nodes_w_keys_.back().second);
            assert(nodes_w_keys_.back().second->Size() == _member_count &&
                   nodes_w_keys_.back().second->Type() == INode::NodeType::Map);

            return EndNode_();
        }

        bool StartArray() { return PutNode_(INode::NodeType::Array); }

        bool EndArray(rapidjson::SizeType _element_count)
        {
            assert(!nodes_w_keys_.empty() && nodes_w_keys_.back().second);
            assert(nodes_w_keys_.back().second->Size() == _element_count &&
                   nodes_w_keys_.back().second->Type() == INode::NodeType::Array);

            return EndNode_();
        }

    private:
        bool PutValue_(XValue&& _val)
        {
            if (nodes_w_keys_.empty()) {
                assert(!root_);
                root_ = std::move(_val);
                return true;
            }

            // Exclude nodes (deserialize at EndNode_)
            if (on_custom_deserialize_ && !_val.QueryPtrC<INode>() &&
                (callback_types_mask_.value_or(_val.Type()) & _val.Type()) == _val.Type()) {
                auto custom_obj_val = on_custom_deserialize_(recent_key_, _val);
                if (custom_obj_val.ObjectPtrC())
                    _val = std::move(custom_obj_val);
            }

            auto [success, insert_at, prev] = nodes_w_keys_.back().second->Insert(
                std::exchange(recent_key_, xnode::kIdxEnd),
                std::move(_val));
            assert(success);
            return success;
        }

        bool PutNode_(const INode::NodeType _node_type)
        {
            auto node_p = XNodeFactoryGet()->NodeCreate(
                _node_type,
                recent_key_.StringGet().value_or(std::string_view {}),
                nodes_w_keys_.empty() ? root_uid_.value_or(xbase::kInvalidUid) : xbase::kInvalidUid);
            PutValue_(node_p);
            nodes_w_keys_.emplace_back(recent_key_, node_p);
            return true;
        }

        bool EndNode_()
        {
            if (nodes_w_keys_.empty())
                return false;

            // Check for custom node
            if (on_custom_deserialize_ && (callback_types_mask_.value_or(XValue::ValueType::kObject) &
                                           XValue::ValueType::kObject) == XValue::ValueType::kObject) {

                // For replace node with deserialized object
                auto parent_sp = nodes_w_keys_.size() > 1 ? nodes_w_keys_[nodes_w_keys_.size() - 2].second :
                                                            root_.QueryPtr<INode>();
                if (parent_sp) {
                    auto custom_obj_val = on_custom_deserialize_(nodes_w_keys_.back().first,
                                                                 nodes_w_keys_.back().second);
                    if (custom_obj_val.ObjectPtrC()) {
                        parent_sp->Set(nodes_w_keys_.back().first, std::move(custom_obj_val));
                    }
                }
            }

            nodes_w_keys_.pop_back();
            return true;
        }
    };

} // namespace impl

xbase::IBuffer::SPtrC xnode::ToJsonBuffer(const XValue&              _root_value,
                                          const OnCustomSerializePf& _pf_on_custom,
                                          const xnode::JsonFormat    _json_format,
                                          const size_t               _indent_char_count,
                                          const char                 _indent_char)
{
    if (!_root_value)
        return {};

    auto buffer = _root_value.QueryPtrC<xbase::IBuffer>();
    if (buffer)
        return buffer;

    auto node_root = _root_value.QueryPtrC<INode>();

    rapidjson::StringBuffer json_buffer;
    if (JsonFormat::kOneLine == _json_format) {
        auto writer_p = std::make_unique<rapidjson::Writer<rapidjson::StringBuffer>>(json_buffer);

        if (node_root)
            impl::WriteXNode(writer_p.get(), node_root.get(), _json_format, _pf_on_custom);
        else
            impl::WriteXValue(writer_p.get(), _root_value, _json_format, _pf_on_custom, {});
    }
    else {
        auto writer_p = std::make_unique<rapidjson::PrettyWriter<rapidjson::StringBuffer>>(json_buffer);
        writer_p->SetFormatOptions(JsonFormat::kOneLineArrays == _json_format ? rapidjson::kFormatSingleLineArray :
                                                                                rapidjson::kFormatDefault);
        writer_p->SetIndent(_indent_char, (uint32_t)_indent_char_count);
        if (node_root)
            impl::WriteXNode(writer_p.get(), node_root.get(), _json_format, _pf_on_custom);
        else
            impl::WriteXValue(writer_p.get(), _root_value, _json_format, _pf_on_custom, {});
    }

    std::string_view sv_data(json_buffer.GetString(), json_buffer.GetSize());
    return xbuffer::CreateStringBuffer(xbase::ToShared(std::move(json_buffer)), std::move(sv_data));
}

std::string xnode::ToJson(const XValue&              _root_value,
                          const OnCustomSerializePf& _pf_on_custom,
                          const xnode::JsonFormat    _json_format,
                          const size_t               _indent_char_count,
                          const char                 _indent_char)
{
    auto buffer_sp = xnode::ToJsonBuffer(_root_value, _pf_on_custom, _json_format, _indent_char_count, _indent_char);
    if (!buffer_sp)
        return {};

    assert(xbuffer::IsString(buffer_sp.get()));
    if (xbuffer::IsString(buffer_sp.get()))
        return std::string(buffer_sp->BufferData());

    return {};
}

std::pair<INode::SPtr, size_t> xnode::FromJson(const std::string_view                 _json,
                                               const std::string_view                 _name,
                                               const xnode::OnCustomDeserializePf&    _on_custom_deserialize,
                                               const std::optional<XValue::ValueType> _callback_types_mask,
                                               const std::optional<xbase::Uid>        _node_uid)
{
    if (_json.empty())
        return {nullptr, -1};

    impl::XNodeJsonHandler handler(_on_custom_deserialize, _callback_types_mask, _node_uid, _name);

    rapidjson::Reader       reader;
    rapidjson::MemoryStream ssInput(_json.data(), _json.size());
    auto                    res       = reader.Parse(ssInput, handler);
    size_t                  error_pos = 0;
    if (res.IsError())
        error_pos = res.Offset() != 0 ? res.Offset() : -1;

    return {handler.root_.QueryPtr<INode>(), error_pos};
}

} // namespace xsdk
