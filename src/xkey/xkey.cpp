#include "xkey/xkey.h"

namespace xsdk {

template <typename TCheck, std::size_t TIndex = 0>
constexpr std::size_t XKeyIndex()
{
    return xbase::VariantIndex<XKeyVariant, TCheck>();
}

bool XKey::IsEmpty() const { return Index_() == XKeyIndex<std::monostate>(); }

XKey::KeyType XKey::Type() const
{
    switch (Index_()) {
        case XKeyIndex<std::monostate>():
            return KeyType::Empty;
        case XKeyIndex<std::size_t>():
            return KeyType::Index;
        case XKeyIndex<std::string_view>():
            return KeyType::String;
    }
    assert(!"Wrong XKeyVariant index");
    return KeyType::Empty;
}

std::optional<size_t> XKey::IndexGet() const { return xbase::VariantGet<size_t>(this); }

std::optional<std::string_view> XKey::StringGet() const { return xbase::VariantGet<std::string_view>(this); }

bool XKey::IsEqual(const std::string_view _compare_with) const
{
    const auto* p_sv = std::get_if<std::string_view>(this);
    if (!p_sv)
        return false;

    return *p_sv == _compare_with;
}

void XKey::InitHolder_()
{
    const auto* p_sv = std::get_if<std::string_view>(this);
    if (p_sv) {
        str_hold_p_          = std::make_shared<const std::string>(*p_sv);
        (XKeyVariant&)* this = std::string_view(*str_hold_p_);
    }
}

} // namespace xsdk