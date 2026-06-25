#include "xvalue/xvalue.h"

#include "xbase/strings.h"

#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <locale>
#include <sstream>

namespace xsdk {

template <typename TCheck>
constexpr std::size_t XValueIndex()
{
    return xbase::VariantIndex<XVariant, TCheck>();
}

XValue& XValue::operator=(const XValue& _val)
{
    (XVariant&)* this = (XVariant&)_val;
    return *this;
}
XValue& XValue::operator=(XValue&& _val) noexcept
{
    (XVariant&)* this = XVariant(std::move(_val));
    return *this;
}

XValue::operator bool() const noexcept
{
    return index() != XValueIndex<std::monostate>() /* && index() != xvalue_index<xv_error>()*/;
}

bool XValue::operator==(const XValue& _val) const
{
    if ((const XVariant&)*this == (const XVariant&)_val)
        return true;

    if (IsInteger() && _val.IsInteger())
        return Int64() == _val.Int64();

    const auto* p_str1 = StringView().data();      //-V758
    const auto* p_str2 = _val.StringView().data(); //-V758
    if (p_str1 && p_str2)                          //-V560 //-V560
        return std::strcmp(p_str1, p_str2) == 0;

    auto sp_obj1 = ObjectPtrC(); //-V779
    auto sp_obj2 = _val.ObjectPtrC();
    if (sp_obj1 && sp_obj2)
        return sp_obj1 == sp_obj2;

    return false;
}

int32_t XValue::Compare(const XValue& _val) const
{
    if ((const XVariant&)*this == (const XVariant&)_val)
        return 0;

    if (IsInteger() && _val.IsInteger())
        return (int32_t)(Int64() - _val.Int64());

    const auto* p_str1 = StringView().data();      //-V758
    const auto* p_str2 = _val.StringView().data(); //-V758
    if (p_str1 && p_str2)                          //-V560 //-V560
        return std::strcmp(p_str1, p_str2);

    auto sp_obj1 = ObjectPtrC(); //-V779
    auto sp_obj2 = _val.ObjectPtrC();
    if (sp_obj1 && sp_obj2)
        return (int32_t)(sp_obj1.get() - sp_obj2.get());

    if ((const XVariant&)*this < (const XVariant&)_val)
        return -1;

    return 1;
}

XValue::ValueType XValue::Type() const noexcept
{
    switch (index()) {
        case XValueIndex<std::monostate>():
            return kEmpty;
        case XValueIndex<XValueNull>():
            return kNull;
        case XValueIndex<IObject::SPtr>():
            assert(std::get<IObject::SPtr>(*this));
            return kObject;
        case XValueIndex<IObject::SPtrC>():
            assert(std::get<IObject::SPtrC>(*this));
            return kConstObject;
        case XValueIndex<bool>():
            return kBool;
        case XValueIndex<int64_t>():
            return kInt64;
        case XValueIndex<uint64_t>():
            return kUint64;
        case XValueIndex<double>():
            return kDouble;
        case XValueIndex<xnode::String::SPtrC>():
            return kString;
    }

    assert(false);
    return kAny;
}

bool XValue::IsObject() const noexcept
{
    if (index() == XValueIndex<IObject::SPtr>() || index() == XValueIndex<IObject::SPtrC>())
        return true;

    return false;
}

bool XValue::IsAttribute() const noexcept
{
    switch (index()) {
        case XValueIndex<bool>():
        case XValueIndex<int64_t>():
        case XValueIndex<uint64_t>():
        case XValueIndex<double>():
        case XValueIndex<xnode::String::SPtrC>():
            return true;

        default:
            return false;
    }
}

bool XValue::IsInteger() const noexcept
{
    if (index() == XValueIndex<int64_t>() || index() == XValueIndex<uint64_t>())
        return true;

    return false;
}

bool XValue::IsNumberConvertable() const noexcept
{
    switch (index()) {
        case XValueIndex<bool>():
        case XValueIndex<int64_t>():
        case XValueIndex<uint64_t>():
        case XValueIndex<double>():
            return true;
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* p_str = std::get_if<xnode::String::SPtrC>(this)->get();
            return IsNumberConvertable_(p_str);
        }
        default:
            return false;
    }
}

void XValue::Reset() { XVariant::emplace<std::monostate>(); }

bool XValue::IsEmpty() const noexcept
{
    switch (index()) {
        case XValueIndex<std::monostate>():
        case XValueIndex<XValueNull>():
            return true;
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* p_str = std::get_if<xnode::String::SPtrC>(this)->get();
            assert(p_str);
            return p_str->empty();
        }
        default:
            return false;
    }
}

bool XValue::Bool(const bool _default) const
{
    switch (index()) {
        case XValueIndex<bool>():
            return std::get<bool>(*this);
        case XValueIndex<int64_t>():
            return std::get<int64_t>(*this) > 0;
        case XValueIndex<uint64_t>():
            return std::get<uint64_t>(*this) > 0;
        case XValueIndex<double>():
            return std::get<double>(*this) > 0.0;
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* p_str = std::get_if<xnode::String::SPtrC>(this)->get();
            assert(p_str);
            // todo: !!! case unsensetive comparision
            return *p_str == "true" || std::atof(p_str->c_str()) > 0;
        }
        default:
            return _default;
    }
}

int64_t XValue::Int64(const int64_t _default) const
{
    switch (index()) {
        case XValueIndex<bool>():
            return std::get<bool>(*this) ? 1 : 0;
        case XValueIndex<int64_t>():
            return std::get<int64_t>(*this);
        case XValueIndex<uint64_t>():
            return xbase::Clamp<int64_t>(std::get<uint64_t>(*this));
            // return (int64_t)std::min(std::get<uint64_t>(*this), (uint64_t)std::numeric_limits<int64_t>::max());
        case XValueIndex<double>():
            return std::llround(std::get<double>(*this));
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* psz = std::get<xnode::String::SPtrC>(*this)->c_str();
            assert(psz);
            char* end = nullptr;
            auto  ll  = std::strtoll(psz, &end, 0);
            return end == psz ? _default : ll;
        }

        default:
            return _default;
    }
}

uint64_t XValue::Uint64(const uint64_t _default, const uint64_t _negative_res) const
{
    switch (index()) {
        case XValueIndex<bool>(): {
            return std::get<bool>(*this) ? 1 : 0;
        }
        case XValueIndex<uint64_t>(): {
            return std::get<uint64_t>(*this);
        }
        case XValueIndex<int64_t>(): {
            auto ll = std::get<int64_t>(*this);
            return ll >= 0 ? (uint64_t)ll : _negative_res;
        }
        case XValueIndex<double>(): {
            auto ll = std::llround(std::get<double>(*this));
            return ll >= 0 ? (uint64_t)ll : _negative_res;
        }
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* psz = std::get<xnode::String::SPtrC>(*this)->c_str();
            assert(psz);
            char* end = nullptr;
            if (psz[0] == '-') {
                auto ll = std::strtoll(psz, &end, 0);
                return ll >= 0 ? (uint64_t)ll : psz == end ? _default : _negative_res;
            }
            auto ull = std::strtoull(psz, &end, 0);
            return end == psz ? _default : ull;
        }
        default:
            return _default;
    }
}

int32_t XValue::Int32(const int32_t _default) const { return xbase::Clamp<int32_t>(Int64(_default)); }

uint32_t XValue::Uint32(const uint32_t _default, const uint32_t _negative_res) const
{
    return xbase::Clamp<uint32_t>(Uint64(_default, _negative_res));
}

double XValue::Double(const double _default) const
{
    switch (index()) {
        case XValueIndex<bool>():
            return std::get<bool>(*this) ? 1.0 : 0.0;
        case XValueIndex<int64_t>():
            return (double)std::get<int64_t>(*this);
        case XValueIndex<uint64_t>():
            return (double)std::get<uint64_t>(*this);
        case XValueIndex<double>():
            return std::get<double>(*this);
        case XValueIndex<xnode::String::SPtrC>(): {
            const auto* p_str = std::get_if<xnode::String::SPtrC>(this)->get();
            if (!IsNumberConvertable_(p_str))
                return _default;
            return std::atof(p_str->c_str());
        }
        default:
            return _default;
    }
}

std::string XValue::String(const std::string_view _default) const
{
    switch (index()) {
        case XValueIndex<bool>():
            return std::get<bool>(*this) ? "true" : "false";
        case XValueIndex<int64_t>():
            return std::to_string(std::get<int64_t>(*this));
        case XValueIndex<uint64_t>():
            return std::to_string(std::get<uint64_t>(*this));
        case XValueIndex<double>():
            return std::to_string(std::get<double>(*this));
        case XValueIndex<xnode::String::SPtrC>():
            return *std::get<xnode::String::SPtrC>(*this);
        default:
            return std::string(_default);
    }
}

std::string_view XValue::StringView(const std::string_view _default) const
{
    const auto* pp_str = std::get_if<xnode::String::SPtrC>(this);
    if (pp_str && *pp_str)
        return *pp_str->get();

    return _default;
}

IObject::SPtr XValue::ObjectPtr(const IObject::SPtr& _default) const
{
    auto pp_obj = std::get_if<IObject::SPtr>(this);
    assert(!pp_obj || *pp_obj);
    if (pp_obj)
        return *pp_obj;

    return _default;
}

IObject::SPtrC XValue::ObjectPtrC(const IObject::SPtrC& _default) const
{
    auto pp_c_obj = std::get_if<IObject::SPtrC>(this);
    assert(!pp_c_obj || *pp_c_obj);
    if (pp_c_obj)
        return *pp_c_obj;

    auto pp_obj = std::get_if<IObject::SPtr>(this);
    assert(!pp_obj || *pp_obj);
    if (pp_obj)
        return *pp_obj;

    return _default;
}

inline bool XValue::IsNumberConvertable_(const std::string* _p_str)
{
    if (!_p_str || !_p_str->length())
        return false;

    const auto* psz = _p_str->c_str();
    // skip spaces
    while (std::isspace(static_cast<uint8_t>(*psz)))
        ++psz;

    // Alow '+', '-', '.'
    if (*psz == '+' || *psz == '-' || *psz == '.')
        ++psz;

    // Have to be digit
    // Note: '.' assumed as invalid
    return std::isdigit(static_cast<uint8_t>(*psz));
}

template <>
std::optional<bool> XValue::OptionalGet<bool>(const std::optional<bool> _default) const
{
    if (IsNumberConvertable())
        return Uint64();

    return _default;
}

template <>
std::optional<float> XValue::OptionalGet<float>(const std::optional<float> _default) const
{
    if (IsNumberConvertable())
        return static_cast<float>(Double());

    return _default;
}

template <>
std::optional<double> XValue::OptionalGet<double>(const std::optional<double> _default) const
{
    if (IsNumberConvertable())
        return Double();

    return _default;
}

#ifdef __APPLE__
template <>
std::optional<size_t> XValue::OptionalGet<size_t>(const std::optional<size_t> _default) const
{
    if (IsNumberConvertable())
        return static_cast<size_t>(Uint64());

    return _default;
}
#endif

template <>
std::optional<int64_t> XValue::OptionalGet<int64_t>(const std::optional<int64_t> _default) const
{
    if (IsNumberConvertable())
        return Int64();

    return _default;
}

template <>
std::optional<uint64_t> XValue::OptionalGet<uint64_t>(const std::optional<uint64_t> _default) const
{
    if (IsNumberConvertable())
        return Uint64();

    return _default;
}

template <>
std::optional<int32_t> XValue::OptionalGet<int32_t>(const std::optional<int32_t> _default) const
{
    if (IsNumberConvertable())
        return xbase::Clamp<int32_t>(Int64());

    return _default;
}

template <>
std::optional<uint32_t> XValue::OptionalGet<uint32_t>(const std::optional<uint32_t> _default) const
{
    if (IsNumberConvertable())
        return xbase::Clamp<uint32_t>(Uint64());

    return _default;
}

template <>
std::optional<int16_t> XValue::OptionalGet<int16_t>(const std::optional<int16_t> _default) const
{
    if (IsNumberConvertable())
        return xbase::Clamp<int16_t>(Int64());

    return _default;
}

template <>
std::optional<uint16_t> XValue::OptionalGet<uint16_t>(const std::optional<uint16_t> _default) const
{
    if (IsNumberConvertable())
        return xbase::Clamp<uint16_t>(Uint64());

    return _default;
}

template <>
std::optional<std::string> XValue::OptionalGet<std::string>(const std::optional<std::string> _default) const
{
    if (Type() == XValue::kString)
        return String();

    return _default;
}

template <>
std::optional<std::string_view> XValue::OptionalGet<std::string_view>(
    const std::optional<std::string_view> _default) const
{
    if (Type() == XValue::kString)
        return StringView();

    return _default;
}

namespace {

    bool ParseBoolExact(const std::string_view _str, bool* const _value_p)
    {
        if (xbase::strings::CmpI(_str, "true") == 0) {
            *_value_p = true;
            return true;
        }

        if (xbase::strings::CmpI(_str, "false") == 0) {
            *_value_p = false;
            return true;
        }

        return false;
    }

    bool ParseIntValue(const std::string_view _str, XValue* const _value_p)
    {
        assert(_value_p);
        if (_str.empty())
            return false;

        const auto*       psz     = _str.data();
        const auto* const psz_end = _str.data() + _str.size();

        // skip spaces
        while (psz <= psz_end && std::isspace(static_cast<uint8_t>(*psz)))
            ++psz;

        // Skip leading '+'
        if (psz < psz_end && *psz == '+')
            ++psz;

        // Check for base 16
        int32_t base = 10;
        if (psz + 2 <= psz_end && psz[0] == '0' && (psz[1] == 'x' || psz[1] == 'X')) {
            base = 16;
            psz += 2;
        }
        else if (psz < psz_end && *psz == '-') {
            int64_t parsed   = {};
            auto [p_end, ec] = std::from_chars(psz, psz_end, parsed, base);
            if (ec != std::errc() || p_end != psz_end)
                return false;

            *_value_p = parsed;
            return true;
        }

        if (psz >= psz_end)
            return false;

        uint64_t parsed  = {};
        auto [p_end, ec] = std::from_chars(psz, psz_end, parsed, base);
        if (ec != std::errc() || p_end != psz_end)
            return false;

        if (parsed <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) && base == 10)
            *_value_p = static_cast<int64_t>(parsed);
        else
            *_value_p = static_cast<uint64_t>(parsed);

        return true;
    }

#ifdef __APPLE__
    bool ParseDoubleExact(const std::string_view _str, double* const _value_p)
    {
        if (!_value_p)
            return false;
        if (_str.empty())
            return false;

        std::istringstream in {std::string(_str)};
        in.imbue(std::locale::classic());

        double parsed = 0.0;
        in >> parsed;

        if (in.fail() || !in.eof())
            return false;

        *_value_p = parsed;
        return true;
    }
#else
    bool ParseDoubleExact(const std::string_view _str, double* const _value_p)
    {
        if (_str.empty())
            return false;

        const auto*       psz     = _str.data();
        const auto* const psz_end = _str.data() + _str.size();

        // skip spaces
        while (psz <= psz_end && std::isspace(static_cast<uint8_t>(*psz)))
            ++psz;

        // Skip leading '+'
        if (psz < psz_end && *psz == '+')
            ++psz;

        if (psz >= psz_end)
            return false;

        double parsed    = 0.0;
        auto [p_end, ec] = std::from_chars(psz, psz_end, parsed);
        if (ec != std::errc() || p_end != psz_end)
            return false;

        *_value_p = parsed;
        return true;
    }
#endif

} // namespace

XValue XValue::FromString(const std::string_view _str)
{
    if (_str.empty())
        return XValue(_str);

    bool bool_value = false;
    if (ParseBoolExact(_str, &bool_value))
        return XValue(bool_value);

    XValue int_value;
    if (ParseIntValue(_str, &int_value))
        return int_value;

    double double_value = 0.0;
    if (ParseDoubleExact(_str, &double_value))
        return XValue(double_value);

    return XValue(_str);
}

} // namespace xsdk
