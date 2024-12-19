#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace xsdk {

// For IContainer INode callback res (?)
/**
 * @brief Enum class representing the possible results of an OnCopyPF callback.
 */
enum class OnCopyRes {
    /// The callback skips the copy operation.
    Skip,
    /// The callback takes the ownership of the copy.
    Take,
    /// The callback takes the ownership of the copy and stops the copying process.
    TakeStop,
    /// The callback stops the copying process.
    Stop
};
/**
 * @brief Enum class representing the possible results of an OnChangePF callback.
 */
enum class OnEachRes {
    /// The callback moves on to the next element.
    Next,
    /// The callback erases the current element and moves on to the next element..
    Erase,
    /// The callback erases the current element and stops the enumeration.
    EraseStop,
    /// The callback stops the enumeration.
    Stop
};

///@name Special values for map/arrays
///@{
static constexpr size_t kIdxBegin = 0;  ///< The index of the first element in the map/array.
static constexpr size_t kIdxEnd   = -1; ///< The index of the one before the last element in the map/array.
static constexpr size_t kIdxLast  = -2; ///< The index of the last element in the map/array.
///@}

///@name Key speration values for allow string representation of XPath
///@{
///@brief Keys delimiter for allow string representation of XPath e.g. "node::inner_node::value"
static constexpr std::string_view kKeyDelimiter = "::"; // Could be switched to "." for have js like style
///@brief Keys speration values for allow string representation of XPath e.g. "node::array_subnode[12]"
static constexpr std::string_view kKeyBraceOpen = "[";
///@brief Keys speration values for allow string representation of XPath e.g. "node::array_subnode[12]"
static constexpr std::string_view kKeyBraceClose = "]";
/**
 * @brief For allow node[subnode]::value access and key names with delimiters
 * @details e.g. node[subnode_name::with_delimiters] -> 'subnode_name::with_delimiters' would be string key
 * @note All between braces [...] if not numbers, assumed as string,
 * including other opening braces, braces NOT COUNTING (could be changed later)
 * e.g. node[subnode_name[with_braces]rest_chars_and_non_counted_brace] ->
 *      "node", "subnode_name[with_braces", "rest_chars_and_non_counted_brace]"
 */
static constexpr bool kStringKeyInBraces = true;
///@}

///@name Special values for XML import/export
///@{
static constexpr std::string_view kXMLAttributePrefix = "-";     ///< Default prefix for XML attributes.
static constexpr std::string_view kXMLValueName       = "#text"; ///< Default name for the XML value element.
///@}

///@name Special values for JSON export
///@{
static constexpr size_t kExportIndentCount = 4;   ///< Default number of spaces for indentation when exporting data.
static constexpr char   kExportIndentChar  = ' '; ///< Default character used for indentation when exporting data.
///@}

} // namespace xsdk
