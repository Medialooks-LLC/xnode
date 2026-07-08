#pragma once

#include "xnode_symbols.h"
#include "xnode_interfaces.h"

#include <memory>
#include <string>
#include <string_view>

namespace xsdk::xnode {

///@name JSON functions
/// Return resulting value
///@{

/**
 * @brief Enum class representing different JSON format options.
 * @details This enum class defines three different JSON format options: kOneLine, kOneLineArrays and kPretty.
 */
enum class JsonFormat {
    /// Pretty JSON format with indentation for readability (default)
    kPretty = 0,

    /// One-line JSON format.
    kOneLine = 1,

    /// Pretty JSON format with arrays represented by a single line.
    kOneLineArrays = 2
};

/**
 * @brief A type alias for a handle custom objects during INode json serialization with folowing signature, return
 * XValue (could be INode inside)
 * @code
 * OnCustomObjectPf = std::function<XValue(const XKey& _key, const IObject* _custom_object)>;
 * @endcode
 */
using OnCustomSerializePf = std::function<XValue(const XKey& _key, const IObject* _custom_object)>;
/**
 * @brief Function to convert an INode object to json format string.
 *
 * @param _root_value         The XValue object to be converted to json format.
 * @param _pf_on_custom        Function pointer to handle custom object serialization.
 * @param _json_format        The desired json format. @see JsonFormat.
 * @param _indent_char_count  Number of characters for indentation <EM> (skipped for one line format)</EM>.
 * @param _indent_char        Character used for indentation <EM> (skipped for one line format)</EM>.
 *
 * @return Returns a IBuffer::Type::StringView buffer containing the json format representation of the INode object.
 */
XNODE_API xbase::IBuffer::SPtrC ToJsonBuffer(const XValue&              _root_value,
                                             const OnCustomSerializePf& _pf_on_custom      = {},
                                             const JsonFormat           _json_format       = JsonFormat::kOneLineArrays,
                                             const size_t               _indent_char_count = kExportIndentCount,
                                             const char                 _indent_char       = kExportIndentChar);
/**
 * @brief Function to convert an INode object to json format string.
 *
 * @param _root_value          The root INode object to be converted to json format.
 * @param _pf_on_custom        Function pointer to handle custom object serialization.
 * @param _json_format        The desired json format. @see JsonFormat.
 * @param _indent_char_count  Number of characters for indentation <EM> (skipped for one line format)</EM>.
 * @param _indent_char        Character used for indentation <EM> (skipped for one line format)</EM>.
 *
 * @return Returns a std::string containing the json format representation of the INode object.
 */
XNODE_API std::string ToJson(const XValue&              _root_value,
                             const OnCustomSerializePf& _pf_on_custom      = {},
                             const JsonFormat           _json_format       = JsonFormat::kOneLineArrays,
                             const size_t               _indent_char_count = kExportIndentCount,
                             const char                 _indent_char       = kExportIndentChar);

/**
 * @brief A type alias for a handle custom objects during INode json deserialization with folowing signature, return
 * XValue (usually with IObject inherited object inside)
 * Called for values depends from types mask, for nodes called then deserializarion finished
 * @code
 * OnCustomDeserializePf = std::function<XValue(const XKey& _key, const IObject* _custom_object)>;
 * @endcode
 */
using OnCustomDeserializePf = std::function<XValue(const XKey& _key, const XValue& _read_value)>;

/**
 * @brief Parses the given JSON string and returns an INode pointer and the error position if any.
 *
 * @param _json The JSON string to be parsed.
 * @param _name The name to be given to the resulting node.
 * @param _on_custom_deserialize The callback for custom objects (e.g. IMediaPacket etc.) deserialization.
 * @param _callback_types_mask mask for types for which deserialize callback should be called.
 * @param _node_uid  The unique identifier for the resulting node.
 *
 * @return A std::pair consisting of an INode pointer and the error position if any.
 *
 * @note A zero error position means that the import from JSON was successful.
 */
XNODE_API std::pair<INode::SPtr, size_t> FromJson(
    const std::string_view                 _json,
    const std::string_view                 _name                  = {},
    const OnCustomDeserializePf&           _on_custom_deserialize = {},
    const std::optional<XValue::ValueType> _callback_types_mask   = {},
    const std::optional<xbase::Uid>        _node_uid              = {});

///@}

} // namespace xsdk::xnode
