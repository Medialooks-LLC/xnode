#pragma once

#include "xnode_interfaces.h"

#include <memory>
#include <string>
#include <string_view>

namespace xsdk::xnode {

///@name JSON functions
/// Return resulting value
///@{
/**
 * @brief Parses the given JSON string and returns an INode pointer and the error position if any.
 *
 * @param _json The JSON string to be parsed.
 * @param _uid  The unique identifier for the resulting node.
 * @param _name The name to be given to the resulting node.
 *
 * @return A std::pair consisting of an INode pointer and the error position if any.
 *
 * @note A zero error position means that the import from JSON was successful.
 */
std::pair<INode::SPtr, size_t> FromJson(std::string_view _json, uint64_t _uid = 0, std::string_view _name = {});

/**
 * @brief Enum class representing different JSON format options.
 * @details This enum class defines three different JSON format options: kOneLine, kOneLineArrays and kPretty.
 */
enum class JsonFormat {
    /// One-line JSON format.
    kOneLine,

    /// Pretty JSON format with arrays represented by a single line.
    kOneLineArrays,

    /// Pretty JSON format with indentation for readability.
    kPretty
};

/**
 * @brief A type alias for a callable object with the following signature:
 * @code
 * OnCopyRes onCopyFunc(const INode::SPtrC& _node_to_copy_from, const XKey& _key, XValueRT& _value)
 * @endcode
 */
using OnCopyPF = const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>;
/**
 * @brief Function to convert an INode object to json format string.
 *
 * @param _node_this          The INode object to be converted to json format.
 * @param _pf_on_item         Function pointer to handle copying of values when recursively traversing the tree.
 * <STRONG>(Currently not imlemented)</STRONG>
 * @param _json_format        The desired json format. @see JsonFormat.
 * @param _indent_char_count  Number of characters for indentation <EM> (skipped for one line format)</EM>.
 * @param _indent_char        Character used for indentation <EM> (skipped for one line format)</EM>.
 *
 * @return Returns a std::string containing the json format representation of the INode object.
 */
std::string ToJson(const INode::SPtrC& _node_this,
                   OnCopyPF&           _pf_on_item        = nullptr,
                   JsonFormat          _json_format       = JsonFormat::kOneLineArrays,
                   size_t              _indent_char_count = kExportIndentCount,
                   char                _indent_char       = kExportIndentChar);

///@}

} // namespace xsdk::xnode
