#pragma once

#include "../xnode_symbols.h"
#include "../xstring.h"
#include "xbase.h"
#include "xconstant.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace xsdk {

/**
 * @brief XValueNull type alias representing null value.
 */
using XValueNull = const void*;

/**
 * @brief XVariant is a variant type template which can store various values: monostate,
 * empty, null, bool, integers, floating-point numbers, strings, and objects.
 */
using XVariant = std::variant<std::monostate,
                              XValueNull,
                              bool,
                              int64_t,
                              uint64_t,
                              double,
                              xnode::String::SPtrC,
                              IObject::SPtrC,
                              IObject::SPtr>;

#ifdef _MSC_VER
    #pragma warning(push)
    // Suppress C4251: private STL members don't need a DLL interface.
    #pragma warning(disable : 4251)
#endif

/**
 * @brief A utility class used for storing and manipulating values.
 * @details It provides a generic variant type with a number of constructor overloads for common types,
 * as well as a set of utility methods to extract and convert these values.
 */
class XNODE_API XValue: protected XVariant {

public:
    /// @brief The XValue enum defines various value types.
    enum /*class*/ ValueType {
        kAny = -1,
        /// Represents an non constant value flag.
        kNonConst = 0x100,

        /// Represents an empty XValue.
        kEmpty = 0x00, // TODO: Rename to kNotInitialized for do not confuse with IsEmpty()
        /// Represents a null XValue.
        kNull = 0x01,
        /// Represents a boolean XValue.
        kBool = 0x02,
        /// Represents a 64-bit integer XValue.
        kInt64 = 0x04,
        /// Represents a 64-bit unsigned integer XValue.
        kUint64 = 0x08,
        /// Represents a double XValue.
        kDouble = 0x10,
        /// Represents a string XValue.
        kString = 0x20,
        /// Represents an object XValue.
        kObject = 0x40 | kNonConst,
        /// Represents a const object XValue.
        kConstObject = kObject & ~kNonConst,
        /// A mask for number types.
        kNumbersMask = kBool | kInt64 | kUint64 | kDouble
    };

public:
    ///@name Base constructors
    ///@{
    /// @brief Move constructor
    XValue(XValue&&) noexcept = default;
    /// @brief Copy constructor
    XValue(const XValue&) = default;

    // xo_empty
    /** @brief Default constructor.*/
    XValue() = default;
    ///@}

    ///@name Null valued constructors
    ///@{
    // xo_null
    /** @brief Constructs an XValue from a null value.*/
    XValue(std::nullptr_t) : XVariant((XValueNull) nullptr) {}
    ///@}

    ///@name Boolean constructors
    ///@{
    // xo_bool
    /** @brief Constructs an XValue from a boolean value.*/
    XValue(bool _val) : XVariant(_val) {}
    ///@}

    ///@name Number constructors
    ///@{
    // xo_int64
    /** @brief Constructs an XValue from a 64-bit integer value.*/
    XValue(int64_t _val) : XVariant(_val) {}
    /// @brief Constructs an XValue from a 32-bit integer value.
    XValue(int32_t _val) : XVariant((int64_t)_val) {}
    /// @brief Constructs an XValue from a 16-bit integer value.
    XValue(int16_t _val) : XVariant((int64_t)_val) {}
    /// @brief Constructs an XValue from a 8-bit integer value.
    XValue(int8_t _val) : XVariant((int64_t)_val) {}

    // xo_uint64
    /** @brief Constructs an XValue from a 64-bit unsigned integer value.*/
    XValue(uint64_t _val) : XVariant(_val) {}
    /// @brief Constructs an XValue from a 32-bit unsigned integer value.
    XValue(uint32_t _val) : XVariant((uint64_t)_val) {}
    /// @brief Constructs an XValue from a 16-bit unsigned integer value.
    XValue(uint16_t _val) : XVariant((uint64_t)_val) {}
    /// @brief Constructs an XValue from a 8-bit unsigned integer value.
    XValue(uint8_t _val) : XVariant((uint64_t)_val) {}

    // size_t fix for MacOS
#ifdef __APPLE__
    /// @brief Constructs an XValue from a size_t value.
    XValue(size_t _val) : XVariant((uint64_t)_val) {}
#endif

    // xo_double
    /** @brief Constructs an XValue from a double value.*/
    XValue(double _val) : XVariant(_val) {}
    ///@}

    ///@name String constructors
    ///@{
    // xo_string
    /** @brief Move constructor an XValue from a std::string value.*/
    XValue(std::string&& _str) : XVariant(std::make_shared<const std::string>(std::move(_str))) {}
    /// @brief Copy constructor an XValue from a std::string value.
    XValue(const std::string& _str) : XVariant(std::make_shared<const std::string>(_str)) {}
    /// @brief Constructor an XValue from a std::string_view object.
    XValue(std::string_view _str) : XVariant(std::make_shared<const std::string>(_str)) {}
    /// @brief Constructor an XValue from a char*.
    XValue(char* _psz) : XVariant(std::make_shared<const std::string>(_psz ? (const char*)_psz : "")) {}
    /// @brief Constructor an XValue from a const char*.
    XValue(const char* _psz) : XVariant(std::make_shared<const std::string>(_psz ? _psz : "")) {}
    ///@}

    ///@name Object constructors
    ///@{
    // xo_object
    /** @brief Move constructor an XValue from a IObject::SPtr.*/
    XValue(IObject::SPtr&& _pXObj) { (XVariant&)* this = _pXObj ? XVariant(std::move(_pXObj)) : XVariant(nullptr); }
    /// @brief Copy constructor an XValue from a IObject::SPtr.
    XValue(const IObject::SPtr& _pXObj) { (XVariant&)* this = _pXObj ? XVariant(_pXObj) : XVariant(nullptr); }
    /** @brief Constructor an XValue from a IObject**/
    XValue(IObject* _obj_p)
    {
        (XVariant&)* this = _obj_p ? XVariant(xobject::PtrQuery<IObject>(_obj_p)) : XVariant(nullptr);
    }
    /// @brief Copy constructor an XValue from a const IObject*.
    XValue(const IObject* _obj_p)
    {
        (XVariant&)* this = _obj_p ? XVariant(xobject::PtrQuery<IObject>(_obj_p)) : XVariant(nullptr);
    }
    /**
     * @brief Constructs an XValue from a shared_ptr of a TObj object.
     * @tparam TObj The object type.
     */
    template <class TObj>
    XValue(std::shared_ptr<TObj>&& _pXObj)
    {
        (XVariant&)* this = _pXObj ? XVariant(std::static_pointer_cast<IObject>(std::move(_pXObj))) : XVariant(nullptr);
    }
    /**
     * @brief Constructs an XValue from a const shared_ptr of a TObj object.
     * @tparam TObj The object type.
     */
    template <class TObj>
    XValue(const std::shared_ptr<TObj>& _pXObj)
    {
        (XVariant&)* this = _pXObj ? XVariant(std::static_pointer_cast<IObject>(_pXObj)) : XVariant(nullptr);
    }

    // xo_const_object
    /** @brief Move constructor an XValue from a IObject::SPtrC.*/
    XValue(IObject::SPtrC&& _pXObj) { (XVariant&)* this = _pXObj ? XVariant(std::move(_pXObj)) : XVariant(nullptr); }
    /// @brief Copy constructor an XValue from a IObject::SPtrC.
    XValue(const IObject::SPtrC& _pXObj) { (XVariant&)* this = _pXObj ? XVariant(_pXObj) : XVariant(nullptr); }
    /**
     * @brief Constructs an XValue from a shared_ptr of a const TObj object.
     * @tparam TObj The object type.
     */
    template <class TObj>
    XValue(std::shared_ptr<const TObj>&& _pXObj)
    {
        (XVariant&)* this = _pXObj ? XVariant(std::static_pointer_cast<const IObject>(std::move(_pXObj))) :
                                     XVariant(nullptr);
    }
    /**
     * @brief Constructs an XValue from a const shared_ptr of a const TObj object.
     * @tparam TObj The object type.
     */
    template <class TObj>
    XValue(const std::shared_ptr<const TObj>& _pXObj)
    {
        (XVariant&)* this = _pXObj ? XVariant(std::static_pointer_cast<const IObject>(_pXObj)) : XVariant(nullptr);
    }
    ///@}

public:
    explicit operator bool() const noexcept;
    bool     operator!() const { return !(bool)*this; }

    XValue& operator=(const XValue& _val);
    XValue& operator=(XValue&& _val) noexcept;

    bool operator==(const XValue& _val) const;
    bool operator!=(const XValue& _val) const { return !(*this == _val); }
    bool operator<(const XValue& _val) const { return Compare(_val) < 0; }
    bool operator<=(const XValue& _val) const { return Compare(_val) <= 0; }
    bool operator>(const XValue& _val) const { return Compare(_val) > 0; }
    bool operator>=(const XValue& _val) const { return Compare(_val) >= 0; }
    /**
     * @brief Compares this XValue with another XValue.
     * @return A negative value if this XValue is less than _val, zero if they are equal, and a positive value if this
     * XValue is greater than _val.
     */
    [[nodiscard]] int32_t Compare(const XValue& _val) const;

    /// @brief Returns the value type of this XValue.
    [[nodiscard]] ValueType Type() const noexcept;
    /// @brief Checks if this XValue is empty (no value, or nullptr, or empty string (?))
    [[nodiscard]] bool IsEmpty() const noexcept;
    /// @brief Checks if this XValue is an object.
    [[nodiscard]] bool IsObject() const noexcept;
    /// @brief Checks if this XValue is an attribute.
    [[nodiscard]] bool IsAttribute() const noexcept;
    /// @brief Checks if this XValue is an integer.
    [[nodiscard]] bool IsInteger() const noexcept;
    /// @brief Checks if this XValue is an number convertable (e.g. numbers and strings).
    [[nodiscard]] bool IsNumberConvertable() const noexcept;

    /// @brief Resets the value of this XValue to its default state (std::monostate).
    void Reset();

    /// @brief Retrieves the boolean value of this XValue.
    /// @note If the XValue type is different from bool, the value will be converted to it.
    [[nodiscard]] bool Bool(const bool _default = false) const;
    /// @brief Retrieves the 64-bit signed integer value of this XValue.
    /// @note If the XValue type is different from 64-bit signed integer, the value will be converted to it.
    [[nodiscard]] int64_t Int64(const int64_t _default = 0) const;
    /// @brief Retrieves the 64-bit unsigned integer value of this XValue.
    /// @note If the XValue type is different from 64-bit unsigned integer, the value will be converted to it.
    /// If resulting value is negative then returned _negative_res value.
    [[nodiscard]] uint64_t Uint64(const uint64_t _default = 0, const uint64_t _negative_res = 0) const;
    /// @brief Retrieves the 32-bit signed integer value of this XValue.
    /// @note If the XValue type is different from 32-bit signed integer, the value will be converted to it.
    [[nodiscard]] int32_t Int32(const int32_t _default = 0) const;
    /// @brief Retrieves the 32-bit unsigned integer value of this XValue.
    /// @note If the XValue type is different from 32-bit unsigned integer, the value will be converted to it.
    /// /// If resulting value is negative then returned _negative_res value.
    [[nodiscard]] uint32_t Uint32(const uint32_t _default = 0, const uint32_t _negative_res = 0) const;
    /// @brief Retrieves the double value of this XValue.
    /// @note If the XValue type is different from double, the value will be converted to it.
    [[nodiscard]] double Double(const double _default = 0.0) const;
    /// @brief Retrieves the string value of this XValue.
    /// @note If the XValue type is different from string, the value will be converted to it.
    [[nodiscard]] std::string String(const std::string_view _default = {}) const;
    /// @brief Retrieves the string_view value of this XValue.
    [[nodiscard]] std::string_view StringView(const std::string_view _default = {}) const;
    /// @brief Retrieves the object pointer of this XValue.
    [[nodiscard]] IObject::SPtr ObjectPtr(const IObject::SPtr& _default = nullptr) const;
    /// @brief Retrieves the object pointer constant of this XValue.
    [[nodiscard]] IObject::SPtrC ObjectPtrC(const IObject::SPtrC& _default = nullptr) const;

    /// @brief Retrieves a shared_ptr of a TObject from this XValue.
    /// @tparam TObject The object type.
    template <typename TObject>
    [[nodiscard]] std::shared_ptr<TObject> QueryPtr(const std::shared_ptr<TObject>& _default = nullptr) const
    {
        auto sp_obj = xobject::PtrQuery<TObject>(ObjectPtr().get());
        return sp_obj ? sp_obj : _default;
    }

    /// @brief Retrieves a shared_ptr of a const TObject from this XValue.
    /// @tparam TObject The object type.
    template <typename TObject>
    [[nodiscard]] std::shared_ptr<const TObject> QueryPtrC(
        const std::shared_ptr<const TObject>& _default = nullptr) const
    {
        auto sp_obj = xobject::PtrQuery<TObject>(ObjectPtrC().get());
        return sp_obj ? sp_obj : _default;
    }

    /// @brief Retrieves a XENUM from this XValue.
    /// @tparam TObject The object type.
    template <typename TEnum>
    [[nodiscard]] std::optional<TEnum> EnumGet(const std::optional<TEnum> _default = {}) const
    {
        return xenum::FromString<TEnum>(String(), _default);
    }

    /// @brief Retrieves a XENUM from this XValue, std::nullopt if not suitable for enum
    template <typename TEnum>
    [[nodiscard]] TEnum EnumGet(const TEnum _default) const
    {
        return xenum::FromString(String(), _default);
    }

    /// @brief Retrieves a specicified optional type from this XValue, if value not suitable, returned
    /// std::nullopt
    template <typename TGet>
    [[nodiscard]] std::optional<TGet> OptionalGet(const std::optional<TGet> _default = {}) const;

    /**
     * @brief Converts a string to the most appropriate scalar XValue type.
     *
     * @param _str Source string.
     * @return Parsed bool / int64_t / uint64_t / double / string value.
     *
     * @details
     * Conversion is attempted in the following order:
     * - bool (`true` / `false`, case-insensitive)
     * - signed integer in decimal form
     * - unsigned integer in decimal form, or hexadecimal form with `0x` prefix
     * - double-precision floating-point value
     * - string if no numeric or boolean conversion succeeds
     *
     * Notes:
     * - quoted-string handling is not performed here;
     * - an empty input remains a string;
     * - integer parsing supports decimal and hexadecimal formats;
     * - hexadecimal values with `0x` prefix are always treated as unsigned integers,
     *   even when the parsed value fits into int64_t;
     * - positive decimal integers that fit into int64_t are returned as int64_t.
     */
    [[nodiscard]] static XValue FromString(const std::string_view _str);

private:
    static bool IsNumberConvertable_(const std::string* _p_str);
};

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// XENUM_OPS32(XValue::ValueType)

template <>
XNODE_API std::optional<bool> XValue::OptionalGet<bool>(const std::optional<bool> _default) const;

template <>
XNODE_API std::optional<double> XValue::OptionalGet<double>(const std::optional<double> _default) const;

#ifdef __APPLE__
template <>
XNODE_API std::optional<size_t> XValue::OptionalGet<size_t>(const std::optional<size_t> _default) const;
#endif

template <>
XNODE_API std::optional<int64_t> XValue::OptionalGet<int64_t>(const std::optional<int64_t> _default) const;

template <>
XNODE_API std::optional<uint64_t> XValue::OptionalGet<uint64_t>(const std::optional<uint64_t> _default) const;

template <>
XNODE_API std::optional<int32_t> XValue::OptionalGet<int32_t>(const std::optional<int32_t> _default) const;

template <>
XNODE_API std::optional<uint32_t> XValue::OptionalGet<uint32_t>(const std::optional<uint32_t> _default) const;

template <>
XNODE_API std::optional<int16_t> XValue::OptionalGet<int16_t>(const std::optional<int16_t> _default) const;

template <>
XNODE_API std::optional<uint16_t> XValue::OptionalGet<uint16_t>(const std::optional<uint16_t> _default) const;

template <>
XNODE_API std::optional<std::string_view> XValue::OptionalGet<std::string_view>(
    const std::optional<std::string_view> _default) const;

template <>
XNODE_API std::optional<std::string> XValue::OptionalGet<std::string>(const std::optional<std::string> _default) const;

} // namespace xsdk
