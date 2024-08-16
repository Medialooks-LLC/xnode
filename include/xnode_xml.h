#pragma once

#include "xconstant.h"
#include "xnode_interfaces.h"

#include <string>
#include <string_view>
#include <system_error>

namespace xsdk::xnode {
///@name XML functions
///@{
/**
 * @brief Initializes the Xerces-C++ platform explicitly.
 * This function initializes the Xerces-C++ platform, which is a prerequisite for working with XML documents.
 * @return Returns true if the initialization was successful, false otherwise.
 */
bool XmlPlatformInit();

/**
 * @brief A function that converts an XML string to an INode object.
 * @param _xml XML string to be parsed.
 * @param _uid The UID of the INode to create, if not provided, a new UID will be generated.
 * @param _name The name of the INode to create, if not provided, the name will be inferred from the XML.
 * @param _attribute_prefix The prefix for XML attributes.
 * @param _value_name Name which will be used as name for XML value (inner text).
 * @return std::pair containing the parsed node and an error line position, if an error occurred.
 * @note If used without a previous call of @ref XmlPlatformInit, lazy initialization will be used.
 */
std::pair<INode::SPtr, size_t> FromXml(std::string_view _xml,
                                       uint64_t         _uid              = 0,
                                       std::string_view _name             = {},
                                       std::string_view _attribute_prefix = kXMLAttributePrefix,
                                       std::string_view _value_name       = kXMLValueName);

/**
 * @brief An enumeration class for specifying the format of the output XML.
 * @details This enum class defines three different XML format options: kOneLine and kPretty.
 */
enum class XmlFormat {
    /// One-line XML format.
    kOneLine,

    /// Pretty XML format with indentation for readability.
    kPretty
};

using OnCopyPF = const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>;
/**
 * @brief A function that converts an INode object to an XML string.
 *
 * @param _node_this The INode object to be converted to XML.
 * @param _pf_on_item A callable object that is invoked for each item in the INode tree that needs to be
 * converted to the output XML. <STRONG>(Currently not imlemented)</STRONG>
 * @param _xml_format The format of the output XML. Can be `kOneLine` or `kPretty`. @ref XmlFormat
 * @param _attribute_prefix The prefix which will define XML attributes.
 * @param _value_name The name of INode element which will be used as XML value (inner text).
 * @param _indent_char_count The number of indentation characters to be used. <STRONG>(Currently not imlemented)</STRONG>
 * @param _indent_char The character to be used for indentation.<STRONG>(Currently not imlemented)</STRONG>
 * @return The resulting XML string.
 * @note If used without a previous call of @ref XmlPlatformInit, lazy initialization will be used.
 */
std::string ToXml(const INode::SPtrC& _node_this,
                  OnCopyPF&           _pf_on_item        = nullptr,
                  XmlFormat           _xml_format        = XmlFormat::kOneLine,
                  std::string_view    _attribute_prefix  = kXMLAttributePrefix,
                  std::string_view    _value_name        = kXMLValueName,
                  size_t              _indent_char_count = kExportIndentCount,
                  char                _indent_char       = kExportIndentChar);

///@}

} // namespace xsdk::xnode