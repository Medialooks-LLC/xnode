#pragma once

#include <cassert>
#include <deque>
#include <string>
#include <string_view>
#include <utility>

#include "xkey.h"

namespace xsdk {

/**
 * @brief A deque (double-ended queue) container for XKey objects representing an XPath
 */
class XPath: public std::deque<XKey> {
public:
    //using deque::deque;
    ///@name Base constructors
    ///@{
    /// @brief Default constructor.
    XPath()                 = default;
    /// @brief Move constructor.
    /// @param _other The other XPath to move from.
    XPath(XPath&& _other) noexcept = default;
    /// @brief Copy constructor.
    /// @param _other The other XPath to copy from.
    XPath(const XPath& _other)     = default;
    ///@}

    ///@name XKey constructors
    ///@{
    /**
     * @brief Explicit constructor taking an XKey object by value and pushing it to the container
     * @param _key The XKey object to add
     */
    explicit XPath(XKey&& _key) { push_back(std::move(_key)); }
    /**
     * @brief Explicit constructor taking an XKey object by const reference and pushing it to the container
     * @param _key The XKey object to add
     */
    explicit XPath(const XKey& _key) { push_back(_key); }
    ///@}

    ///@name Constructors from strings
    ///@{
    /**
     * @brief Constructor taking a null-terminated character string and creating XPath keys from it
     * @param _str The null-terminated character string
     * @note The "::" is used as keys delimeter in string
     */
    XPath(const char* _str) { _add_keys(_str); }
    //XPath(char* _str) { _add_keys(_str); }
    /**
     * @brief Constructor taking a std::string object and creating XPath keys from it
     * @param _str The std::string object to create keys from
     * @note The "::" is used as keys delimeter in string
     */
    XPath(const std::string& _str) { _add_keys(_str); }
    ///@}

    ///@name Constructors with variable argument list
    ///@{
    /**
     * @brief Constructor with variable argument list
     * @param _idx The first key to add
     * @tparam TArgs Variadic template arguments representing keys to add
     */
    template <typename... TArgs>
    XPath(size_t _idx, TArgs&&... _args)
    {
        _add_keys(_idx);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _str The std::string_view to create first keys from
     * @tparam TArgs Variadic template arguments representing keys to add
     */
    template <typename... TArgs>
    XPath(std::string_view _str, TArgs&&... _args)
    {
        _add_keys(_str);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    ///@}

    // template <typename... TArgs>
    // XPath(const std::string& _str, TArgs&&... _args)
    //{
    //     _add_keys(_str);
    //     (_add_keys(std::forward<TArgs>(_args)), ...);
    // }

public:
    /**
     * @brief Empty XKey object.
     */
    inline static XKey empty_key;

    /**
     * @brief Returns a reference to the first XKey in the XPath container.
     * @return Reference to the first XKey in the container if it is not empty, otherwise the empty_key.
     */
    XKey&       front() { return empty() ? empty_key : std::deque<XKey>::front(); }
    /**
     * @brief Returns a const reference to the first XKey in the XPath container.
     * @return Const reference to the first XKey in the container if it is not empty, otherwise the empty_key as a const reference
     */
    const XKey& front() const { return empty() ? empty_key : std::deque<XKey>::front(); }

    /**
     * @brief Returns a reference to the last XKey in the XPath container.
     * @return Reference to the last XKey in the container if it is not empty, otherwise the empty_key.
     */
    XKey&       back() { return empty() ? empty_key : std::deque<XKey>::back(); }
    /**
     * @brief Returns a const reference to the last XKey in the XPath container.
     * @return Const reference to the last XKey in the container if it is not empty, otherwise the empty_key as a const reference.
     */
    const XKey& back() const { return empty() ? empty_key : std::deque<XKey>::back(); }

    /**
     * @brief Remove and return the last XKey from the XPath container.
     * @return The last XKey before removal.
     */
    XKey pop_back();
    /**
     * @brief Remove and return the first XKey from the XPath container.
     * @return The first XKey before removal.
     */
    XKey pop_front();
    /**
     * @brief For allow to have flat nodes, string access to node 
     * @return string reperesetation of path e.g. "node::array_subnode[12]::value"
     */
    std::string to_string() const;

private:
    static std::pair<XKeyVariant, std::string_view> _split_key(std::string_view _str);

    void _add_keys(size_t _idx) { push_back(XKey(_idx)); }
    void _add_keys(std::string_view _str) { _add_keys_str(std::string(_str)); }
    void _add_keys_str(std::string&& _str);
};

} // namespace xsdk