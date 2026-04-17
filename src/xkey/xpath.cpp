#include "xkey/xpath.h"

#include <iostream>
#include <sstream>

namespace xsdk {

XKey& XPath::Front()
{
    assert(!Empty());
    return Empty() ? kEmptyKey : path_.front();
}
XKey& XPath::Back()
{
    assert(!Empty());
    return Empty() ? kEmptyKey : path_.back();
}
XKey XPath::PopBack()
{
    assert(!path_.empty());
    if (path_.empty())
        return kEmptyKey;
    XKey key = path_.back();
    path_.pop_back();
    return key;
}
XKey XPath::PopFront()
{
    assert(!path_.empty());
    if (path_.empty())
        return kEmptyKey;
    XKey key = path_.front();
    path_.pop_front();
    return key;
}

std::string XPath::ToString() const
{
    std::ostringstream out;
    for (const auto& key : path_) {
        if (key.Type() == XKey::KeyType::String) {
            // check for path with dots and braces
            auto key_str = key.StringGet().value();
            if (key_str.find(xnode::kKeyDelimiter) != std::string_view::npos ||
                key_str.find(xnode::kKeyBraceClose) != std::string_view::npos ||
                key_str.find(xnode::kKeyBraceOpen) != std::string_view::npos) {
                // 2Think:  use ['string'] ?
                out << xnode::kKeyBraceOpen << key_str << xnode::kKeyBraceClose;
            }
            else {
                out << (out.tellp() != 0 ? xnode::kKeyDelimiter : "") << key_str;
            }
        }
        else if (key.Type() == XKey::KeyType::Index) {
            out << xnode::kKeyBraceOpen << key.IndexGet().value() << xnode::kKeyBraceClose;
        }
    }

    return std::move(out).str();
}

bool XPath::IsPrefix(const XPath& _check_for_prefix) const
{
    if (path_.size() > _check_for_prefix.Size())
        return false;

    for (size_t z = 0; z < path_.size(); ++z)
        if (path_.at(z) != _check_for_prefix.At(z))
            return false;

    return true;
}

XPath XPath::Subpath(const size_t _start, const size_t _len) const
{
    auto start = std::min(_start, path_.size());
    auto len   = std::min(_len, path_.size() - start);

    return XPath(std::deque<XKey> {path_.begin() + start, path_.begin() + start + len});
}

XPath XPath::PopFront(const size_t _elements)
{
    std::deque<XKey> removed;
    auto             elements = _elements;
    while (elements-- > 0 && !path_.empty()) {
        removed.push_back(std::move(path_.front()));
        path_.pop_front();
    }
    return XPath {std::move(removed)};
}

bool XPath::operator<(const XPath& _other) const
{
    return std::lexicographical_compare(path_.begin(), path_.end(), _other.path_.begin(), _other.path_.end());
}

bool XPath::operator==(const XPath& _other) const
{
    if (path_.size() != _other.Size())
        return false;

    for (size_t z = 0; z < path_.size(); ++z)
        if (path_.at(z) != _other.At(z))
            return false;

    return true;
}

bool XPath::operator!=(const XPath& _other) const { return !(_other == (*this)); }

const XKey& XPath::At(size_t _idx) const
{
    if (_idx >= path_.size())
        return kEmptyKey;
    return path_.at(_idx);
}

/*static*/ std::pair<XKeyVariant, std::string_view> XPath::_split_key(std::string_view _str)
{
    assert(!_str.empty());
    auto pos_dots = _str.find(xnode::kKeyDelimiter);
    if (pos_dots == 0)
        return _split_key(_str.substr(xnode::kKeyDelimiter.length())); // Fix for do not have empty keys

    // Check for begining from brace
    auto pos_brace = _str.find(xnode::kKeyBraceOpen);
    if (pos_brace == 0 && _str.length() > xnode::kKeyBraceOpen.length()) {
        // for opened brace the end of key is ']' - for allow to have keys with dots e.g. [allow::have::dots]
        auto pos_end = _str.find(xnode::kKeyBraceClose);

        // check for index e.g. [123]
        if (!xnode::kStringKeyInBraces || std::isdigit(static_cast<unsigned char>(_str[1]))) {
            // Do not expect index more than max_int
            size_t key_idx = (size_t)std::atoi(_str.data() + 1);
            if (pos_end == std::string_view::npos || pos_end + xnode::kKeyBraceClose.length() >= _str.length())
                return {key_idx, {}};

            return {key_idx, _str.substr(pos_end + xnode::kKeyBraceClose.length())};
        }

        // 2Think: support for ['string key']

        // Take string e.g. [something::inside]
        auto key_str = _str.substr(xnode::kKeyBraceOpen.length(), pos_end - xnode::kKeyBraceOpen.length());
        if (pos_end == std::string_view::npos || pos_end + xnode::kKeyBraceClose.length() >= _str.length())
            return {key_str, {}};

        return {key_str, _str.substr(pos_end + xnode::kKeyBraceClose.length())};
    }

    if (pos_brace < pos_dots) {
        if (pos_brace + xnode::kKeyBraceOpen.length() >= _str.length())
            return {_str, {}};

        // Next part have to be started from '['
        return {_str.substr(0, pos_brace), _str.substr(pos_brace)};
    }

    if (pos_dots == std::string_view::npos || pos_dots + xnode::kKeyDelimiter.length() >= _str.length())
        return {_str.substr(0, pos_dots), {}};

    return {_str.substr(0, pos_dots), _str.substr(pos_dots + xnode::kKeyDelimiter.length())};
}

void XPath::_add_keys(XPath&& _path)
{
    path_.insert(path_.end(), std::make_move_iterator(_path.path_.begin()), std::make_move_iterator(_path.path_.end()));
    _path.path_.clear();
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
                path_.emplace_back(str_hold_p, std::move(key));

            str = next_str;
        }
    }
}

} // namespace xsdk