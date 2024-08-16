#include "xnode.h"
#include "xnode_functions.h"
#include "xnode_xml.h"

#ifdef _DEBUG
    #include "xnode_json.h"
#endif

#include <gtest/gtest.h>

using namespace xsdk;

// NOLINTBEGIN(*)
TEST(xnode_xml_export_unit_tests, check_invalid_value)
{
    auto res = xnode::ToXml(nullptr);
    EXPECT_EQ("", res);
}

TEST(xnode_xml_export_unit_tests, check_valid_map_node_simple)
{
    auto node = xnode::CreateMap({{"-a", "b"}, {kXMLValueName, "abc"}}, "root");
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root a="b">abc</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_simlpe_format)
{
    auto node = xnode::CreateMap({{"-a", "b"}, {kXMLValueName, "abc"}}, "root");
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><root a="b">abc</root>)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_map_node_simple2)
{
    auto node = xnode::CreateMap({{"-root", "b"}, {kXMLValueName, "abc"}}, "root");
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root root="b">abc</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_map_node_with_nested_map)
{
    auto node         = xnode::CreateMap({{"-a", "b"}}, "root");
    auto nested_node1 = xnode::CreateMap({{"-d", "e"}}, "c");
    nested_node1->ParentSet(node);
    auto nested_node2  = xnode::CreateMap({{"-i", "j"}}, "f");
    auto nested_node21 = xnode::CreateMap({{"-l", "m"}}, "k");
    nested_node21->ParentSet(nested_node2);
    node->Set("f", nested_node2);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root a="b">
  <c d="e"/>
  <f i="j">
    <k l="m"/>
  </f>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_array_node_simple)
{
    auto node       = xnode::CreateMap({}, "root");
    auto array_node = xnode::CreateArray({"0", "1", 3}, "arr");
    array_node->ParentSet(node);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root>
  <arr>0</arr>
  <arr>1</arr>
  <arr>3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_array_node_with_attrs)
{
    auto node       = xnode::CreateMap({{"-a", "b"}}, "root");
    auto array_node = xnode::CreateArray({}, "arr");
    array_node->ParentSet(node);

    auto el1 = xnode::CreateMap({{"-name", "one"}, {kXMLValueName, 1}});
    array_node->Insert(kIdxEnd, el1);
    auto el2 = xnode::CreateMap({{"-value", 33}, {kXMLValueName, 2}});
    array_node->Insert(kIdxEnd, el2);
    auto el3 = xnode::CreateMap({{"-islast", true}, {kXMLValueName, 3}});
    array_node->Insert(kIdxEnd, el3);

#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root a="b">
  <arr name="one">1</arr>
  <arr value="33">2</arr>
  <arr islast="true">3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_array_node_nested_array)
{
    auto node       = xnode::CreateMap({}, "root");
    auto array_node = xnode::CreateArray({1, 222, 3}, "arr");
    array_node->ParentSet(node);

    auto nested_array_node = xnode::CreateArray({"2.1", "2.2", "2.3"}, "bar");
    array_node->Set(1, nested_array_node);

#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root>
  <arr>1</arr>
  <arr>
    <bar>2.1</bar>
    <bar>2.2</bar>
    <bar>2.3</bar>
  </arr>
  <arr>3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_array_node_with_attrs2)
{
    auto node       = xnode::CreateMap({{"-a", "b"}}, "root");
    auto array_node = xnode::CreateArray({}, "arr");
    array_node->ParentSet(node);

    auto el1 = xnode::CreateMap({{"-name", "one"}, {kXMLValueName, 1}});
    array_node->Insert(kIdxEnd, el1);
    auto el2 = xnode::CreateMap({{"-value", 33}, {kXMLValueName, 2}}, "bla");
    array_node->Insert(kIdxEnd, el2);
    auto el3 = xnode::CreateMap({{"-islast", true}, {kXMLValueName, 3}});
    array_node->Insert(kIdxEnd, el3);

#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root a="b">
  <arr name="one">1</arr>
  <arr>
    <bla value="33">2</bla>
  </arr>
  <arr islast="true">3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_valid_array_node_nested_map)
{
    auto node       = xnode::CreateMap({}, "root");
    auto array_node = xnode::CreateArray({}, "arr");
    array_node->ParentSet(node);

    // auto el1 = xnode::CreateMap({{"name", "one"}, {kXMLValueName, 1}});
    array_node->Insert(kIdxEnd, 1);
    auto array_node2 = xnode::CreateArray({2});

    auto el2_1 = xnode::CreateMap({{kXMLValueName, "2.1"}}, "foo");
    array_node2->Insert(kIdxEnd, el2_1);

    auto el2_2 = xnode::CreateMap({{kXMLValueName, "2.2"}}, "bar");
    array_node2->Insert(kIdxEnd, el2_2);

    array_node->Insert(kIdxEnd, array_node2);

    // auto el3 = xnode::CreateMap({{"islast", true}, {kXMLValueName, 3}});
    array_node->Insert(kIdxEnd, 3);

#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root>
  <arr>1</arr>
  <noname>2</noname>
  <noname>
    <foo>2.1</foo>
  </noname>
  <noname>
    <bar>2.2</bar>
  </noname>
  <arr>3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion)
{
    auto str = R"(<root><arr>1</arr><arr>2<foo>2.1</foo><bar>2.2</bar></arr><arr>3</arr></root>)";
    // { "arr": ["1", ["2", { "foo": "2.1" }, { "bar": "2.2" }], "3"] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root>
  <arr>1</arr>
  <arr>2
    <foo>2.1</foo>
    <bar>2.2</bar>
  </arr>
  <arr>3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion2)
{
    auto str = R"(<root a="b">abc<c d="e"/>def</root>)";
    // { "#text": [ "abc", {"-d" : "e"}, "def" ], "-a": "b" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><root a="b">abc<c d="e"/>def</root>)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion3)
{
    auto str = R"(<root a="b"><inn>abc<c d="e"/>def</inn></root>)"; //problem with inn has or not has attrs
    // { "-a": "b", "inn": [ "abc", {"-d" : "e"}, "def" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><root a="b"><inn>abc<c d="e"/>def</inn></root>)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion4)
{
    auto str = R"(<root a="b">some <i>italic text</i> and <b>bold text</b>.</root>)";
    // { "#text": [ "some ", {"i" : "italic text"}, " and ", {"b" : "bold text"}, "." ], "-a": "b" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><root a="b">some <i>italic text</i> and <b>bold text</b>.</root>)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion5)
{
    auto str = R"(<root><arr>1</arr><foo>2.1</foo><bar>2.2</bar><arr>3</arr></root>)";
    // { "arr": [ "1", {"foo" : "2.1"}, {"bar" : "2.2"}, "3" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node, nullptr, xnode::XmlFormat::kPretty);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<root>
  <arr>1</arr>
  <arr>
    <foo>2.1</foo>
  </arr>
  <arr>
    <bar>2.2</bar>
  </arr>
  <arr>3</arr>
</root>
)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion6)
{
    auto str = R"(<alice>bob<charlie>david</charlie>edgar</alice>)";
    // {  "alice": [ "bob", {"charlie" : "david"}, "edgar"] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><alice>bob<charlie>david</charlie>edgar</alice>)";

    EXPECT_EQ(ref, res);
}

TEST(xnode_xml_export_unit_tests, check_double_conversion_with_unicode)
{
    auto str =
        R"(<?xml version="1.0" encoding="utf-8"?><root attr="alphabet алфавит 字母 ← ⇐">alphabet алфавит 字母 ← ⇐&#x1f642;</root>)";
    //{ "#text": "alphabet алфавит 字母 ← ⇐🙂", "-attr": "alphabet алфавит 字母 ← ⇐" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    auto res = xnode::ToXml(node);

    auto ref = R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?><root attr="alphabet алфавит 字母 ← ⇐">alphabet алфавит 字母 ← ⇐🙂</root>)";

    EXPECT_EQ(ref, res);
}
// NOLINTEND(*)
