#pragma once

#include "xconstant.h"
#include "xbase.h"
#include "../xstring.h"

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
using XVariant = std::
    variant<std::monostate, XValueNull, bool, int64_t, uint64_t, double, xnode::String::SPtrC, IObject::SPtrC, IObject::SPtr>;

/**
* @brief A utility class used for storing and manipulating values.
* @details It provides a generic variant type with a number of constructor overloads for common types,
* as well as a set of utility methods to extract and convert these values.
*/
class XValue: protected XVariant {

public:
    /// @brief The XValue enum defines various value types.
    enum ValueType {
        kAny      = -1,
        /// Represents an non constant value flag.
        kNonConst = 0x100,

        /// Represents an empty XValue.
        kEmpty       = 0x00,
        /// Represents a null XValue.
        kNull        = 0x01,
        /// Represents a boolean XValue.
        kBool        = 0x02,
        /// Represents a 64-bit integer XValue.
        kInt64       = 0x04,
        /// Represents a 64-bit unsigned integer XValue.
        kUint64      = 0x08,
        /// Represents a double XValue.
        kDouble      = 0x10,
        /// Represents a string XValue.
        kString      = 0x20,
        /// Represents an object XValue.
        kObject      = 0x40 | kNonConst,
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
    XValue(const XValue&)     = default;

    // xo_empty
    /** @brief Default constructor.*/
    XValue() = default;
    ///@}

    ///@name Null valued constructors
    ///@{
    // xo_null
    /** @brief Constructs an XValue from a null value.*/
    XValue(std::nullptr_t) : XVariant((XValueNull) nullptr) {}
    /// @brief Constructs an XValue from a null value.
    XValue(const void*) : XVariant((XValueNull) nullptr) {}
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

    bool    operator==(const XValue& _val) const;
    bool    operator!=(const XValue& _val) const { return !(*this == _val); }
    bool    operator<(const XValue& _val) const { return Compare(_val) < 0; }
    bool    operator<=(const XValue& _val) const { return Compare(_val) <= 0; }
    bool    operator>(const XValue& _val) const { return Compare(_val) > 0; }
    bool    operator>=(const XValue& _val) const { return Compare(_val) >= 0; }
    /**
     * @brief Compares this XValue with another XValue.
     * @return A negative value if this XValue is less than _val, zero if they are equal, and a positive value if this
     * XValue is greater than _val.
     */
    int32_t Compare(const XValue& _val) const;

    /// @brief Returns the value type of this XValue.
    ValueType Type() const noexcept;
    /// @brief Checks if this XValue is empty.
    bool      IsEmpty() const noexcept;
    /// @brief Checks if this XValue is an object.
    bool      IsObject() const noexcept;
    /// @brief Checks if this XValue is an attribute.
    bool      IsAttribute() const noexcept;
    /// @brief Checks if this XValue is an integer.
    bool      IsInteger() const noexcept;

    /// @brief Resets the value of this XValue to its default state (std::monostate).
    void Reset();

    /// @brief Retrieves the boolean value of this XValue.
    /// @note If the XValue type is different from bool, the value will be converted to it.
    bool             Bool(bool _default = false) const;
    /// @brief Retrieves the 64-bit signed integer value of this XValue.
    /// @note If the XValue type is different from 64-bit signed integer, the value will be converted to it.
    int64_t          Int64(int64_t _default = 0) const;
    /// @brief Retrieves the 64-bit unsigned integer value of this XValue.
    /// @note If the XValue type is different from 64-bit unsigned integer, the value will be converted to it.
    /// If resulting value is negative then returned _negative_res value.
    uint64_t         Uint64(uint64_t _default = 0, uint64_t _negative_res = 0) const;
    /// @brief Retrieves the 32-bit signed integer value of this XValue.
    /// @note If the XValue type is different from 32-bit signed integer, the value will be converted to it.
    int32_t          Int32(int32_t _default = 0) const;
    /// @brief Retrieves the 32-bit unsigned integer value of this XValue.
    /// @note If the XValue type is different from 32-bit unsigned integer, the value will be converted to it.
    /// /// If resulting value is negative then returned _negative_res value.
    uint32_t         Uint32(uint32_t _default = 0, uint32_t _negative_res = 0) const;
    /// @brief Retrieves the double value of this XValue.
    /// @note If the XValue type is different from double, the value will be converted to it.
    double           Double(double _default = 0.0) const;
    /// @brief Retrieves the string value of this XValue.
    /// @note If the XValue type is different from string, the value will be converted to it.
    std::string      String(std::string_view _default = {}) const;
    /// @brief Retrieves the string_view value of this XValue.
    std::string_view StringView(std::string_view _default = {}) const;
    /// @brief Retrieves the object pointer of this XValue.
    IObject::SPtr    ObjectPtr(IObject::SPtr _default = nullptr) const;
    /// @brief Retrieves the object pointer constant of this XValue.
    IObject::SPtrC   ObjectPtrC(IObject::SPtrC _default = nullptr) const;

    /// @brief Retrieves a shared_ptr of a TObject from this XValue.
    /// @tparam TObject The object type.
    template <typename TObject>
    std::shared_ptr<TObject> QueryPtr(std::shared_ptr<TObject> _default = nullptr) const
    {
        auto sp_obj = xobject::PtrQuery<TObject>(ObjectPtr().get());
        return sp_obj ? sp_obj : _default;
    }

    /// @brief Retrieves a shared_ptr of a const TObject from this XValue.
    /// @tparam TObject The object type.
    template <typename TObject>
    std::shared_ptr<const TObject> QueryPtrC(std::shared_ptr<const TObject> _default = nullptr) const
    {
        auto sp_obj = xobject::PtrQuery<TObject>(ObjectPtrC().get());
        return sp_obj ? sp_obj : _default;
    }
};

} // namespace xsdk