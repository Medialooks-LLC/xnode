#include "xvalue/xvalue.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>

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
        case XValueIndex<xnode::String::SPtrC>():
            return true;

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
        case XValueIndex<xnode::String::SPtrC>():
            if (!std::get<xnode::String::SPtrC>(*this))
                assert(std::get<xnode::String::SPtrC>(*this));
            return std::get<xnode::String::SPtrC>(*this)->empty();

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
            auto p_str = std::get<xnode::String::SPtrC>(*this);
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

int32_t XValue::Int32(const int32_t _default) const
{
    return xbase::Clamp<int32_t>(Int64(_default));
    // auto ll = Int64(_default);
    // return ll < std::numeric_limits<int32_t>::min() ? std::numeric_limits<int32_t>::min() :
    //        ll > std::numeric_limits<int32_t>::max() ? std::numeric_limits<int32_t>::max() :
    //                                                   static_cast<int32_t>(ll);
}

uint32_t XValue::Uint32(const uint32_t _default, const uint32_t _negative_res) const
{
    return xbase::Clamp<uint32_t>(Uint64(_default, _negative_res));
    // auto ull = Uint64(_default, _negative_res);
    // return ull > std::numeric_limits<uint32_t>::max() ? std::numeric_limits<uint32_t>::max() :
    //                                                     static_cast<uint32_t>(ull);
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
        case XValueIndex<xnode::String::SPtrC>():
            return std::atof(std::get<xnode::String::SPtrC>(*this)->c_str());
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

template <>
std::optional<bool> XValue::OptionalGet<bool>() const
{
    if (IsNumberConvertable())
        return Uint64();

    return std::nullopt;
}

template <>
std::optional<float> XValue::OptionalGet<float>() const
{
    if (IsNumberConvertable())
        return static_cast<float>(Double());

    return std::nullopt;
}

template <>
std::optional<double> XValue::OptionalGet<double>() const
{
    if (IsNumberConvertable())
        return Double();

    return std::nullopt;
}

template <>
std::optional<int64_t> XValue::OptionalGet<int64_t>() const
{
    if (IsNumberConvertable())
        return Int64();

    return std::nullopt;
}

template <>
std::optional<uint64_t> XValue::OptionalGet<uint64_t>() const
{
    if (IsNumberConvertable())
        return Uint64();

    return std::nullopt;
}

template <>
std::optional<int32_t> XValue::OptionalGet<int32_t>() const
{
    if (IsNumberConvertable())
        return xbase::Clamp<int32_t>(Int64());

    return std::nullopt;
}

template <>
std::optional<uint32_t> XValue::OptionalGet<uint32_t>() const
{
    if (IsNumberConvertable())
        return xbase::Clamp<uint32_t>(Uint64());

    return std::nullopt;
}

template <>
std::optional<int16_t> XValue::OptionalGet<int16_t>() const
{
    if (IsNumberConvertable())
        return xbase::Clamp<int16_t>(Int64());

    return std::nullopt;
}

template <>
std::optional<uint16_t> XValue::OptionalGet<uint16_t>() const
{
    if (IsNumberConvertable())
        return xbase::Clamp<uint16_t>(Uint64());

    return std::nullopt;
}

template <>
std::optional<std::string> XValue::OptionalGet<std::string>() const
{
    if (Type() == XValue::kString)
        return String();

    return std::nullopt;
}

template <>
std::optional<std::string_view> XValue::OptionalGet<std::string_view>() const
{
    if (Type() == XValue::kString)
        return StringView();

    return std::nullopt;
}

} // namespace xsdk