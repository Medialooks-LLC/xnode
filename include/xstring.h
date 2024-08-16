#pragma once

#include "xbase.h"

#include <memory>
#include <string>
#include <string_view>

namespace xsdk::xnode {

/**
 * @brief This class is a wrapper around std::string that simplifies the allocation and deallocation process.
 */
class String: public xbase::PtrBase<std::string> {
public:
    /**
     * @brief Allocates a new std::string instance from a string_view.
     * @param  _str The std::string_view to wrap.
     * @return A unique_ptr pointing to the newly allocated std::string instance, or nullptr if the string_view is empty.
     */
    static UPtrC Alloc(std::string_view _str)
    {
        return _str.empty() ? nullptr : std::make_unique<const std::string>(_str);
    }
    /**
     * @brief Allocates a new std::string instance from an existing string pointer.
     * @param  _str_p An existing C++ string pointer to wrap.
     * @return A unique_ptr pointing to the newly allocated std::string instance, or nullptr if the string pointer is empty.
     */
    static UPtrC Alloc(const std::string* _str_p)
    {
        return (!_str_p || _str_p->empty()) ? nullptr : std::make_unique<const std::string>(*_str_p);
    }
};

} // namespace xsdk::xnode
