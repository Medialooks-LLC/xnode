#include "xnode_functions.h"

#include "xbase.h"

#include <string_view>
#include <utility>
#include <vector>

namespace xsdk {
namespace {

    // TODO: Move to xbase::ParamStringItem or ParamParseResult
    bool WasQuotedValue(const std::string_view _source, const xbase::ParamTokenView& _value)
    {
        if (_value.begin == 0)
            return false;
        if (_value.end >= _source.size())
            return false;

        const char quote_open  = _source[_value.begin - 1];
        const char quote_close = _source[_value.end];

        if (quote_open != quote_close)
            return false;

        return quote_open == '\'' || quote_open == '"';
    }

} // namespace

std::pair<INode::SPtr, xbase::ParamParseResult> xnode::ParseParamString(const std::string_view _param_str,
                                                                        const bool             _keep_string_values,
                                                                        const XValue           _value_for_flags)
{
    auto node = xnode::CreateMap();

    auto parsed = xbase::ParseParamString(_param_str, xbase::ParamParseOptions(true));
    if (parsed.Empty())
        return {node, std::move(parsed)};

    for (const auto& item : parsed.items) {
        if (_keep_string_values || WasQuotedValue(parsed.SourceView(_param_str), item.value))
            xnode::Set(node, XPath(item.key.text), XValue(item.value.text));
        else
            xnode::Set(node, XPath(item.key.text), XValue::FromString(item.value.text));
    }

    if (_value_for_flags.Type() != XValue::kEmpty) {
        for (const auto& flag : parsed.flags)
            xnode::Set(node, XPath(flag.key.text), XValue(_value_for_flags));
    }

    return {node, std::move(parsed)};
}

} // namespace xsdk
