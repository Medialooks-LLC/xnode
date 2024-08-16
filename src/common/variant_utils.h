#pragma once

#include <memory>
#include <optional>
#include <variant>

namespace xsdk::xnode::impl {

// utility method
template <class T, class... Types>
constexpr std::optional<T> VariantGet(const std::variant<Types...>* _var_p)
{
    const auto* p = std::get_if<T>(_var_p);
    if (!p)
        return std::nullopt;

    return *p;
}

// https://stackoverflow.com/questions/52303316/get-index-by-type-in-stdvariant
template <typename TVariant, typename TCheck, std::size_t TIndex = 0>
constexpr std::size_t VariantIndex()
{
    static_assert(std::variant_size_v<TVariant> > TIndex, "type not found in TVariant");
    if constexpr (TIndex == std::variant_size_v<TVariant>)
        return TIndex;
    else if constexpr (std::is_same_v<std::variant_alternative_t<TIndex, TVariant>, TCheck>)
        return TIndex;
    else
        return VariantIndex<TVariant, TCheck, TIndex + 1>();
}

} // namespace xsdk::xbase