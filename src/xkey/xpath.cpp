#include "xkey/xpath.h"

#include <iostream> 
#include <sstream>

namespace xsdk {

XKey XPath::pop_back()
{
    if (empty())
        return empty_key;

    XKey key = std::deque<XKey>::back();
    std::deque<XKey>::pop_back();
    return key;
}

XKey XPath::pop_front()
{
    if (empty())
        return empty_key;

    XKey key = std::deque<XKey>::front();
    std::deque<XKey>::pop_front();
    return key;
}

std::string XPath::to_string() const
{
    std::ostringstream out;
    for (const auto& key : *this) {
        if (key.Type() == XKey::KeyType::String) {
            // check for path with dots and braces
            auto key_str = key.StringGet().value();
            if (key_str.find(kKeyDelimiter) != std::string_view::npos ||
                key_str.find(kKeyBraceClose) != std::string_view::npos ||
                key_str.find(kKeyBraceOpen) != std::string_view::npos) {
                // 2Think:  use ['string'] ?
                out << kKeyBraceOpen << key_str << kKeyBraceClose;
            }
            else {
                out << (out.tellp() != 0 ? kKeyDelimiter : "") << key_str;
            }
        }
        else if (key.Type() == XKey::KeyType::Index) {
            out << kKeyBraceOpen << key.IndexGet().value() << kKeyBraceClose;
        }
    }

    return std::move(out).str();
}

/*static*/ std::pair<XKeyVariant, std::string_view> XPath::_split_key(std::string_view _str)
{
    assert(!_str.empty());
    auto pos_dots  = _str.find(kKeyDelimiter);
    if (pos_dots == 0)
        return _split_key(_str.substr(kKeyDelimiter.length())); // Fix for do not have empty keys

    // Check for begining from brace
    auto pos_brace = _str.find(kKeyBraceOpen);
    if (pos_brace == 0 && _str.length() > kKeyBraceOpen.length()) {
        // for opened brace the end of key is ']' - for allow to have keys with dots e.g. [allow::have::dots]
        auto pos_end = _str.find(kKeyBraceClose);
        
        // check for index e.g. [123]
        if (!kStringKayInBraces || std::isdigit(_str[1])) {
            // Do not expect index more than max_int
            size_t key_idx = (size_t)std::atoi(_str.data() + 1);
            if (pos_end == std::string_view::npos || pos_end + kKeyBraceClose.length() >= _str.length())
                return {key_idx, {}};

            return {key_idx, _str.substr(pos_end + kKeyBraceClose.length())};
        }

        // 2Think: support for ['string key']

        // Take string e.g. [something::inside]
        auto key_str = _str.substr(kKeyBraceOpen.length(), pos_end - kKeyBraceOpen.length());
        if (pos_end == std::string_view::npos || pos_end + kKeyBraceClose.length() >= _str.length())
            return {key_str, {}};

        return {key_str, _str.substr(pos_end + kKeyBraceClose.length())};
    }

    if (pos_brace < pos_dots)
    {
        if (pos_brace + kKeyBraceOpen.length() >= _str.length())
            return {_str, {}};

         // Next part have to be started from '[' 
         return {_str.substr(0, pos_brace), _str.substr(pos_brace)};
    }
    
    if (pos_dots == std::string_view::npos || pos_dots + kKeyDelimiter.length() >= _str.length())
        return {_str.substr(0, pos_dots), {}};

    return {_str.substr(0, pos_dots), _str.substr(pos_dots + kKeyDelimiter.length())};
}

void XPath::_add_keys_str(std::string&& _str)
{
    if (!_str.empty()) {
        xnode::String::SPtrC str_hold_p = std::make_shared<const std::string>(std::move(_str));
        std::string_view     str        = *str_hold_p;
        while (!str.empty()) {
            auto [key, next_str] = _split_key(str);
            const auto* p_str    = std::get_if<std::string_view>(&key);
            if (!p_str || !p_str->empty())
                emplace_back(str_hold_p, std::move(key));

            str = next_str;
        }
    }
}

} // namespace xsdk