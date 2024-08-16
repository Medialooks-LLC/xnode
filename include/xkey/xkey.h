#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "../xconstant.h"
#include "../xstring.h"
#include "xbase.h"

namespace xsdk {

/**
 * @brief Defines an alias for @c std::variant with @c std::monostate, @c size_t, and @c std::string_view types
 */
using XKeyVariant = std::variant<std::monostate, size_t, std::string_view>;

// 2Think: maybe just use std::variant<...std::string> ?
/**
 * @brief The XKey class is a variant wrapper to store different types of keys.
 *
 * The XKey class is a wrapper of @c std::variant which can hold @c std::monostate, @c size_t,
 * @c std::string_view as key.
 */
class XKey: public XKeyVariant {
    /**
     * @brief Holder for the owned string.
     */
    xnode::String::SPtrC str_hold_p_;

public:
    ///@name Base constructors
    ///@{
    /**
     * @brief Default constructor.
     */
    XKey()                = default;
    /**
     * @brief Move constructor.
     * @param _other The other XKey to move from.
     */
    XKey(XKey&& _other) noexcept = default;
    /**
     * @brief Copy constructor.
     * @param _other The other XKey to copy from.
     */
    XKey(const XKey& _other) = default;
    ///@}

    ///@name XKeyVariant constructors
    ///@{
    /**
     * @brief Move constructor from XKeyVariant.
     * @param _var The XKeyVariant to initialize the XKey from.
     */
    XKey(XKeyVariant&& _var) : XKeyVariant(std::move(_var)) { InitHolder_(); }
    /**
     * @brief Copy constructor from XKeyVariant.
     * @param _var The XKeyVariant to initialize the XKey from.
     */
    XKey(const XKeyVariant& _var) : XKeyVariant(_var) { InitHolder_(); }
    ///@}

    ///@name Constructors from numbers
    ///@{
    /**
     * @brief Constructor from int64_t.
     * @param _idx The int64_t to initialize the XKey with.
     */
    XKey(int64_t _idx) : XKeyVariant((size_t)_idx) {}
    /**
     * @brief Constructor from int32_t.
     * @param _idx The int32_t to initialize the XKey with.
     */
    XKey(int32_t _idx) : XKeyVariant((size_t)_idx) {}
    /**
     * @brief Constructor from uint32_t.
     * @param _idx The uint32_t to initialize the XKey with.
     */
    XKey(uint32_t _idx) : XKeyVariant((size_t)_idx) {}
    /**
     * @brief Constructor from size_t.
     * @param _idx The size_t to initialize the XKey with.
     */
    XKey(size_t _idx) : XKeyVariant(_idx) {}
    ///@}

    ///@name Constructors from strings
    ///@{
    /**
     * @brief Constructor from std::string_view.
     * @param _str The std::string_view to initialize the XKey with.
     */
    XKey(std::string_view _str) : XKeyVariant(_str) { InitHolder_(); }
    /**
     * @brief Move constructor from std::string.
     * @param _str The std::string to initialize the XKey with.
     */
    XKey(std::string&& _str)
    {
        str_hold_p_           = std::make_shared<const std::string>(std::move(_str));
        (XKeyVariant&)* this = *str_hold_p_;
    }
    /**
     * @brief Copy constructor from std::string.
     * @param _str The std::string to initialize the XKey with.
     */
    XKey(const std::string& _str) : XKeyVariant(std::string_view(_str)) { InitHolder_(); }
    /**
     * @brief Constructor from char*.
     * @param _psz The char* to initialize the XKey with.
     */
    XKey(char* _psz) : XKeyVariant(std::string_view(_psz)) { InitHolder_(); }
    /**
     * @brief Constructor from const char*.
     * @param _psz The const char* to initialize the XKey with.
     */
    XKey(const char* _psz) : XKeyVariant(std::string_view(_psz)) { InitHolder_(); }
    ///@}

    ///@name Constructors with custom holder
    ///@{
    /**
     * @brief Constructor from @c XKeyVariant with custom string holder.
     * @param _str_hold_p The @c xbase::String::SPtrC to store the XKey string with.
     * @param _var The @c XKeyVariant to initialize the XKey with.
     */
    XKey(const xnode::String::SPtrC& _str_hold_p, XKeyVariant&& _var)
        : XKeyVariant(std::move(_var)),
          str_hold_p_(_str_hold_p)
    {
    }
    /**
     * @brief Constructor from @c std::string_view with custom string holder.
     * @param _str_hold_p The @c xbase::String::SPtrC to store the XKey string with.
     * @param _str The @c std::string_view to initialize the XKey with.
     */
    XKey(const xnode::String::SPtrC& _str_hold_p, std::string_view _str) : XKeyVariant(_str), str_hold_p_(_str_hold_p) {}
    ///@}

    XKey& operator=(XKey&&)      = default;
    XKey& operator=(const XKey&) = default;

public:
    explicit operator bool() const { return !IsEmpty(); }
    bool     operator!() const { return IsEmpty(); }
    /**
     * @brief Check if the XKey is empty.
     * @return Returns true if the XKey is empty, otherwise false.
     */
    bool     IsEmpty() const;

    /**
     * @brief The type of the XKey.
     * @return The KeyType enumeration value representing the type of the XKey.
     */
    enum class KeyType { 
        /// @brief Represents an empty key.
        Empty,

        /// @brief Represents a key that holds an index value.
        Index,

        /// @brief Represents a key that holds a string value.
        String };
    /**
     * @brief Getter method for retrieving the type of the XKey.
     *
     * @return The KeyType of the XKey.
     */
    KeyType                         Type() const;

    /**
     * @brief Get the index of the XKey if it is an index type.
     * @return std::optional<size_t> containing the index value or std::nullopt if the XKey is not an index type.
     */
    std::optional<size_t>           IndexGet() const;
    /**
     * @brief Get the @c std::string_view of the XKey if it is a string type.
     * @return std::optional<std::string_view> containing the string view value or std::nullopt if the XKey is
     * not a string type.
     */
    std::optional<std::string_view> StringGet() const;

private:
    void InitHolder_();

    // Make private for hide XKeyVariant::index() from public access
    // - could easy mess 'index' with 'IndexGet' and got error.
    size_t Index_() const { return XKeyVariant::index(); }
};

} // namespace xsdk