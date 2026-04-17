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
class XPath {
    std::deque<XKey> path_;

    // Empty XKey object.
    inline static XKey kEmptyKey;

public:
    // using deque::deque;
    ///@name Base constructors
    ///@{
    /// @brief Default constructor.
    XPath() = default;
    /// @brief Move constructor.
    /// @param _other The other XPath to move from.
    XPath(XPath&& _other) noexcept = default;
    /// @brief Copy constructor.
    /// @param _other The other XPath to copy from.
    XPath(const XPath& _other) = default;
    ///@}

    ///@name XKey constructors
    ///@{
    /// @brief constructor from XKey deques
    /// @param _other The other XKey deques to move from.
    XPath(std::deque<XKey>&& _other) noexcept : path_(std::move(_other)) {}
    /**
     * @brief Explicit constructor taking an XKey object by value and pushing it to the container
     * @param _key The XKey object to add
     */
    explicit XPath(XKey&& _key) : path_ {std::move(_key)} {}
    /**
     * @brief Explicit constructor taking an XKey object by const reference and pushing it to the container
     * @param _key The XKey object to add
     */
    explicit XPath(const XKey& _key) : path_ {_key} {}

    ///@}

    ///@name Constructors from strings
    ///@{
    /**
     * @brief Constructor taking a null-terminated character string and creating XPath keys from it
     * @param _str The null-terminated character string
     * @note The "::" is used as keys delimeter in string
     */
    XPath(const char* _str) { _add_keys(_str); }
    /**
     * @brief Constructor taking a std::string_view object and creating XPath keys from it
     * @param _str The std::string_view object to create keys from
     * @note The "::" is used as keys delimeter in string
     */
    XPath(const std::string_view _str) { _add_keys(_str); }
    /**
     * @brief Constructor taking a std::string object and creating XPath keys from it
     * @param _str The std::string object to create keys from
     * @note The "::" is used as keys delimeter in string
     */
    XPath(const std::string& _str) { _add_keys(_str); }
    /**
     * @brief Constructor taking a std::string object and creating XPath keys from it
     * @param _str The std::string object to create keys from
     * @note The "::" is used as keys delimeter in string
     */
    XPath(std::string&& _str) { _add_keys_str(std::move(_str)); }
    ///@}

    ///@name Constructors with variable argument list
    ///@{
    /**
     * @brief Constructor with variable argument list
     * @param _idx The first key to add
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    explicit XPath(size_t _idx, TArgs&&... _args)
    {
        _add_keys(_idx);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _str The std::string_view to create first keys from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    XPath(std::string_view _str, TArgs&&... _args)
    {
        _add_keys(_str);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _str The const char* to create first keys from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    XPath(const char* _str, TArgs&&... _args)
    {
        _add_keys(_str);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _str The std::string to create first keys from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    XPath(const std::string& _str, TArgs&&... _args)
    {
        _add_keys(_str);
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _str The std::string to create first keys from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    XPath(std::string&& _str, TArgs&&... _args)
    {
        _add_keys_str(std::move(_str));
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _path The XPath to create base path from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    explicit XPath(const XPath& _path, TArgs&&... _args) : path_(_path.path_)
    {
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    /**
     * @brief Constructor with variable argument list
     * @param _path The XPath to create base path from
     * @tparam TArgs Variadic template arguments representing keys or paths to add
     */
    template <typename... TArgs>
    explicit XPath(XPath&& _path, TArgs&&... _args) : path_(std::move(_path.path_))
    {
        (_add_keys(std::forward<TArgs>(_args)), ...);
    }
    ///@}


public:
    /**
     * @brief access to undelying std::vector object for enumerate parts.
     */
    const std::deque<XKey>& Parts() const { return path_; }

    /**
     * @brief Check that path is empty
     */
    bool Empty() const { return path_.empty(); }

    /**
     * @brief Returns a reference to the first XKey in the XPath container.
     * @return Reference to the first XKey in the container if it is not empty, otherwise the empty_key.
     */
    XKey& Front();
    /**
     * @brief Returns a const reference to the first XKey in the XPath container.
     * @return Const reference to the first XKey in the container if it is not empty, otherwise the empty_key as a
     * const reference
     */
    const XKey& Front() const { return Empty() ? kEmptyKey : path_.front(); }

    /**
     * @brief Returns a reference to the last XKey in the XPath container.
     * @return Reference to the last XKey in the container if it is not empty, otherwise the empty_key.
     */
    XKey& Back();

    /**
     * @brief Returns a const reference to the last XKey in the XPath container.
     * @return Const reference to the last XKey in the container if it is not empty, otherwise the empty_key as a
     * const reference.
     */
    const XKey& Back() const { return Empty() ? kEmptyKey : path_.back(); }

    /**
     * @brief Remove and return the last XKey from the XPath container.
     * @return The last XKey before removal.
     */
    XKey PopBack();

    /**
     * @brief Remove and return the first XKey from the XPath container.
     * @return The first XKey before removal.
     */
    XKey PopFront();

    /**
     * @brief Prepends the given XKey to the beginning of the XPath container.
     * @param The XKey which will be inserted.
     */
    void PushFront(XKey&& _key) { path_.push_front(std::move(_key)); }

    /**
     * @brief Prepends the given XKey to the beginning of the XPath container.
     * @param The XKey which will be inserted.
     */
    void PushFront(const XKey& _key) { path_.emplace_front(_key); }

    /**
     * @brief Prepends the given XKey to the end of the XPath container.
     * @param The XKey which will be inserted.
     */
    void PushBack(XKey&& _key) { path_.push_back(std::move(_key)); }

    /**
     * @brief Prepends the given XKey to the end of the XPath container.
     * @param The XKey which will be inserted.
     */
    void PushBack(const XKey& _key) { path_.emplace_back(_key); }

    /**
     * @brief For allow to have flat nodes, string access to node
     * @return string reperesetation of path e.g. "node::array_subnode[12]::value"
     */
    std::string ToString() const;
    /**
     * @brief check is this path prefix for given string
     */
    bool IsPrefix(const XPath& _check_for_prefix) const;
    /**
     * @brief return the subpath of this path
     */
    XPath Subpath(const size_t _start, const size_t _len = xbase::npos) const;
    /**
     * @brief Pop _elements from front of path
     */
    XPath PopFront(const size_t _elements);

    /**
     * @brief default assigment operators
     */
    XPath& operator=(XPath&& other)      = default;
    XPath& operator=(const XPath& other) = default;
    /**
     * @brief less operator - for ability to XPath as std::map key
     * @return true if this less than other
     */
    bool operator<(const XPath& other) const;
    /**
     * @brief equal operator
     * @return true if equal with other
     */
    bool operator==(const XPath& other) const;
    /**
     * @brief not equal operator
     * @return true if not equal with other
     */
    bool operator!=(const XPath& other) const;

    /**
     * @brief Get number of elements of the path
     */
    size_t Size() const { return path_.size(); }

    /**
     * @brief Get element by index in the path
     * @param _idx index of element
     * @return The element by index in the path or empty_element if index higher then size of path.
     */
    const XKey& At(size_t _idx) const;

private:
    static std::pair<XKeyVariant, std::string_view> _split_key(std::string_view _str);

    void _add_keys(const XPath& _path) { path_.insert(path_.end(), _path.path_.begin(), _path.path_.end()); }
    void _add_keys(XPath&& _path);
    void _add_keys(const size_t _idx) { path_.emplace_back(_idx); }
    void _add_keys(const std::string_view _str) { _add_keys_str(std::string(_str)); }
    void _add_keys(const std::string& _str) { _add_keys_str(std::string(_str)); }
    void _add_keys(std::string&& _str) { _add_keys_str(std::move(_str)); }
    void _add_keys(const char* _str) { _add_keys_str(std::string(_str)); }
    void _add_keys_str(std::string&& _str);
};

} // namespace xsdk