#include "xnode.h"
#include "xnode_xml.h"

#ifdef _DEBUG
    #include "xnode_json.h"
#endif

#include <gtest/gtest.h>
#include <thread>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_xml_import_unit_tests, check_invalid_xml_non_xml_text)
{
    auto str           = R"(It's some text, but it's not XML at all!)";
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node == nullptr);
    EXPECT_EQ(1001, e_pos);
}

TEST(xnode_xml_import_unit_tests, check_invalid_xml_without_root)
{
    auto str           = R"(<a>1</a><b>2</b>)";
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node == nullptr);
    EXPECT_EQ(1009, e_pos);
}

TEST(xnode_xml_import_unit_tests, check_invalid_xml_double_attribute)
{
    auto str           = R"(<root a="a" b="b" a="c">asd</root>)";
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node == nullptr);
    EXPECT_EQ(1025, e_pos);
}

TEST(xnode_xml_import_unit_tests, check_invalid_xml_non_valid_symbols)
{
    auto str           = R"(<?xml version="1.0" encoding="ascii"?><root>字母</root>)";
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node == nullptr);
    EXPECT_EQ(1039, e_pos);
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_simple)
{
    auto str           = R"(<root>abc</root>)";
    auto [node, e_pos] = xnode::FromXml(str);

    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("root");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("abc", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_simple_with_attr)
{
    auto str = R"(<root a="b">abc</root>)";
    // { "-a": "b", "#text": "abc"}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    val = node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("abc", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_simple_with_attr2)
{
    auto str = R"(<root root="def">abc</root>)";
    // { "-root": "def", "#text": "abc"}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-root");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("def", val.String());
    val = node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("abc", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_with_nested_map)
{
    auto str = R"(<root a="b"><c d="e"/><f i="j"><k>l</k></f></root>)";
    // { "-a": "b", "c": {"-d" : "e"}, "f": { "-i": "j", "k": { "k": "l" } } }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    // c node
    val = node->At("c");
    EXPECT_FALSE(val.IsEmpty());
    auto c_node = val.QueryPtr<INode>();
    EXPECT_TRUE(c_node);
    val = c_node->At("-d");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("e", val.String());
    // f node
    val = node->At("f");
    EXPECT_FALSE(val.IsEmpty());
    auto f_node = val.QueryPtr<INode>();
    EXPECT_TRUE(f_node);
    val = f_node->At("-i");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("j", val.String());
    // k node
    val = f_node->At("k");
    EXPECT_FALSE(val.IsEmpty());
    auto k_node = val.QueryPtr<INode>();
    EXPECT_TRUE(k_node);
    val = k_node->At("k");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("l", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_array_node_simple)
{
    auto str = R"(<root><arr>0</arr><arr>1</arr><arr>2</arr></root>)";
    //{ "arr": [ "0", "1", "2" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    EXPECT_EQ(3, arr_node->Size());
    for (int64_t i = 0; i < 3; i++) {
        val = arr_node->At(i);
        EXPECT_FALSE(val.IsEmpty());
        EXPECT_EQ(i, val.Int64());
    }
}

TEST(xnode_xml_import_unit_tests, check_valid_array_node_with_attrs)
{
    auto str = R"(<root a="b"><arr name="one">1</arr><arr value="33">2</arr><arr islast="true">3</arr></root>)";
    // { "-a": "b", "arr": [ {"#text" : "1", "-name" : "one"}, {"#text" : "2", "-value" : "33"}, {"#text" : "3",
    // "-islast" : "true"} ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    EXPECT_EQ(3, arr_node->Size());
    val = arr_node->At(0);
    EXPECT_FALSE(val.IsEmpty());
    if (val) {
        auto arr_node = val.QueryPtr<INode>();
        val           = arr_node->At(kXMLValueName);
        EXPECT_EQ(1, val.Int64());
        val = arr_node->At("-name");
        EXPECT_EQ("one", val.String());
    }
    val = arr_node->At(1);
    EXPECT_FALSE(val.IsEmpty());
    if (val) {
        auto arr_node = val.QueryPtr<INode>();
        val           = arr_node->At(kXMLValueName);
        EXPECT_EQ(2, val.Int64());
        val = arr_node->At("-value");
        EXPECT_EQ(33, val.Int64());
    }
    val = arr_node->At(2);
    EXPECT_FALSE(val.IsEmpty());
    if (val) {
        auto arr_node = val.QueryPtr<INode>();
        val           = arr_node->At(kXMLValueName);
        EXPECT_EQ(3, val.Int64());
        val = arr_node->At("-islast");
        EXPECT_TRUE(val.Bool());
    }
}

TEST(xnode_xml_import_unit_tests, check_valid_array_node_nested_array)
{
    auto str = R"(<root><arr>1</arr><arr>2<bar>2.1</bar><bar>2.2</bar><bar>2.3</bar></arr><arr>3</arr></root>)";
    // { "arr": [ "1", [ "2", "2.1", "2.2", "2.3" ], "3" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    EXPECT_EQ(3, arr_node->Size());
    for (int64_t i = 0; i < 3; i += 2) {
        val = arr_node->At(i);
        EXPECT_FALSE(val.IsEmpty());
        EXPECT_EQ(i + 1, val.Int64());
    }
    val = arr_node->At(1); // [ "2", "2.1", "2.2", "2.3" ]
    EXPECT_FALSE(val.IsEmpty());
    if (val) {
        auto bar_node_value = val.QueryPtr<INode>();
        EXPECT_TRUE(bar_node_value);
        EXPECT_TRUE(bar_node_value->Type() == INode::NodeType::Array);
        EXPECT_TRUE(bar_node_value->IsName("bar"));
        for (int64_t i = 0; i < 4; i++) {
            val = bar_node_value->At(i);
            EXPECT_EQ(2 + i / 10., val.Double());
        }
    }
}

TEST(xnode_xml_import_unit_tests, check_valid_array_node_nested_array2)
{
    auto str = R"(<root><arr>1</arr><arr><bar>2.1</bar><bar>2.2</bar><bar>2.3</bar></arr><arr>3</arr></root>)";
    // { "arr": [ "1", {"bar" : [ "2.1", "2.2", "2.3" ]}, "3" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    EXPECT_EQ(3, arr_node->Size());
    for (int64_t i = 0; i < 3; i += 2) {
        val = arr_node->At(i);
        EXPECT_FALSE(val.IsEmpty());
        EXPECT_EQ(i + 1, val.Int64());
    }
    val = arr_node->At(1); // { bar: [ "2.1", "2.2", "2.3" ]}
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node2 = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node2);
    val = arr_node2->At("bar");
    if (val) {
        auto bar_node = val.QueryPtr<INode>();
        EXPECT_TRUE(bar_node);
        EXPECT_TRUE(bar_node->Type() == INode::NodeType::Array);
        for (int64_t i = 0; i < 3; i++) {
            val = bar_node->At(i);
            EXPECT_EQ(2 + (i + 1) / 10., val.Double());
        }
    }
}

TEST(xnode_xml_import_unit_tests, check_valid_array_node_nested_map)
{
    auto str = R"(<root><arr>1</arr><arr>2<foo>2.1</foo><bar>2.2</bar></arr><arr>3</arr></root>)";
    // { "arr": ["1", ["2", { "foo": "2.1" }, { "bar": "2.2" }], "3"] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    EXPECT_EQ(INode::NodeType::Array, arr_node->Type());
    EXPECT_EQ(3, arr_node->Size());
    for (int64_t i = 0; i < 3; i += 2) {
        val = arr_node->At(i);
        EXPECT_FALSE(val.IsEmpty());
        EXPECT_EQ(i + 1, val.Int64());
    }
    val = arr_node->At(1); // ["2", { "foo": "2.1" }, { "bar": "2.2" }]
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node2 = val.QueryPtr<INode>();
    val            = arr_node2->At(0);
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(2, val.Int64());
    val = arr_node2->At(1); // { "foo": "2.1" }
    EXPECT_FALSE(val.IsEmpty());
    auto foo_node = val.QueryPtr<INode>();
    EXPECT_TRUE(foo_node->IsName("foo"));
    val = foo_node->At("foo");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(2.1, val.Double());
    val = arr_node2->At(2); // { "bar": "2.2" }
    EXPECT_FALSE(val.IsEmpty());
    auto bar_node = val.QueryPtr<INode>();
    EXPECT_TRUE(bar_node->IsName("bar"));
    val = bar_node->At("bar");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(2.2, val.Double());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_node_in_text)
{
    auto str = R"(<root a="b">abc<c d="e"/>def</root>)";
    // { "#text": [ "abc", {"-d" : "e"}, "def" ], "-a": "b" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    val = node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_EQ(INode::NodeType::Array, arr_node->Type());
    val = arr_node->At(0); // "abc"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("abc", val.String());
    val = arr_node->At(1); // {"d" : "e"}
    EXPECT_FALSE(val.IsEmpty());
    auto c_node = val.QueryPtr<INode>();
    EXPECT_TRUE(c_node->IsName("c"));
    val = c_node->At("-d");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("e", val.String());
    val = arr_node->At(2); // "def"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("def", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_node_in_text2)
{
    auto str = R"(<root a="b"><inn>abc<c d="e"/>def</inn></root>)";
    // { "-a": "b", "inn": { "#text": [ "abc", {"-d" : "e"}, "def" ] } }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    val = node->At("inn"); // "inn": { "#text": [ "abc", {"-d" : "e"}, "def" ] }
    EXPECT_FALSE(val.IsEmpty());
    auto inn_node = val.QueryPtr<INode>();
    val           = inn_node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_EQ(INode::NodeType::Array, arr_node->Type());
    val = arr_node->At(0); // "abc"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("abc", val.String());
    val = arr_node->At(1); // {"d" : "e"}
    EXPECT_FALSE(val.IsEmpty());
    auto c_node = val.QueryPtr<INode>();
    EXPECT_TRUE(c_node->IsName("c"));
    val = c_node->At("-d");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("e", val.String());
    val = arr_node->At(2); // "def"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("def", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_node_in_text3)
{
    auto str = R"(<root a="b">some <i>italic text</i> and <b>bold text</b>.</root>)";
    // { "#text": [ "some ", {"i" : "italic text"}, " and ", {"b" : "bold text"}, "." ], "-a": "b" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("b", val.String());
    val = node->At(kXMLValueName); // [ "some ", {"i" : "italic text"}, " and ", {"b" : "bold text"}, "." ]
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_EQ(INode::NodeType::Array, arr_node->Type());
    val = arr_node->At(0); // "some "
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("some ", val.String());
    val = arr_node->At(1); // {"i" : "italic text"}
    EXPECT_FALSE(val.IsEmpty());
    auto i_node = val.QueryPtr<INode>();
    EXPECT_TRUE(i_node->IsName("i"));
    val = i_node->At("i");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("italic text", val.String());
    val = arr_node->At(2); // " and "
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(" and ", val.String());
    val = arr_node->At(3); // {"b" : "bold text"}
    EXPECT_FALSE(val.IsEmpty());
    auto b_node = val.QueryPtr<INode>();
    EXPECT_TRUE(b_node->IsName("b"));
    val = b_node->At("b");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("bold text", val.String());
    val = arr_node->At(4); // "."
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(".", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_node_in_text4)
{
    auto str = R"(<alice>bob<charlie>david</charlie>edgar</alice>)";
    // {  "alice": [ "bob", {"charlie" : "david"}, "edgar"] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("alice"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("alice"); // [ "bob", {"charlie" : "david"}, "edgar"]
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    val           = arr_node->At(0); // "bob"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("bob", val.String());
    val = arr_node->At(1); // {"charlie" : "david"}
    EXPECT_FALSE(val.IsEmpty());
    auto charlie_node = val.QueryPtr<INode>();
    EXPECT_TRUE(charlie_node->IsName("charlie"));
    val = charlie_node->At("charlie");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("david", val.String());
    val = arr_node->At(2); // "edgar"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("edgar", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_node_in_text5)
{
    auto str = R"(<root><alice>bob</alice><alice charlie="david"></alice><alice>edgar</alice></root>)";
    // { "alice": [ "bob", {"-charlie" : "david"}, "edgar" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("alice"); // [ "bob", {"charlie" : "david"}, "edgar" ]
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    val           = arr_node->At(0); // "bob"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("bob", val.String());
    val = arr_node->At(1); // {"charlie" : "david"}
    EXPECT_FALSE(val.IsEmpty());
    auto charlie_node = val.QueryPtr<INode>();
    EXPECT_TRUE(charlie_node->IsName("alice"));
    val = charlie_node->At("-charlie");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("david", val.String());
    val = arr_node->At(2); // "edgar"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("edgar", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_dirty_array)
{
    auto str = R"(<root><a>bob</a><a>charlie</a><b>david</b><a>edgar</a></root>)";
    // { "a": [ "bob", "charlie", {"b" : "david"}, "edgar" ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("a"); // [ "bob", "charlie", {"b" : "david"}, "edgar" ]
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    val           = arr_node->At(0); // "bob"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("bob", val.String());
    val = arr_node->At(1); // "charlie"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("charlie", val.String());
    val = arr_node->At(2); // {"b" : "david"}
    EXPECT_FALSE(val.IsEmpty());
    auto b_node = val.QueryPtr<INode>();
    EXPECT_TRUE(b_node->IsName("b"));
    val = b_node->At("b");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("david", val.String());
    val = arr_node->At(3); // "edgar"
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("edgar", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_dirty_array2)
{
    auto str = R"(<root><a x="1"/><b x="2"/><a x="3"/><c x="4"/></root>)";
    // { "a": [ "1", {"-x" : "2"}, "3", {"-x" : "4"} ] }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("a");
    EXPECT_FALSE(val.IsEmpty());
    auto a_node = val.QueryPtr<INode>();
    val         = a_node->At(0);
    EXPECT_FALSE(val.IsEmpty());
    auto v_node = val.QueryPtr<INode>();
    val         = v_node->At("-x");
    EXPECT_EQ("1", val.String());

    val = a_node->At(1);
    EXPECT_FALSE(val.IsEmpty());
    v_node = val.QueryPtr<INode>();
    EXPECT_TRUE(v_node->IsName("b"));
    val = v_node->At("-x");
    EXPECT_EQ("2", val.String());

    val = a_node->At(2);
    EXPECT_FALSE(val.IsEmpty());
    v_node = val.QueryPtr<INode>();
    val    = v_node->At("-x");
    EXPECT_EQ("3", val.String());

    val = a_node->At(3);
    EXPECT_FALSE(val.IsEmpty());
    v_node = val.QueryPtr<INode>();
    EXPECT_TRUE(v_node->IsName("c"));
    val = v_node->At("-x");
    EXPECT_EQ("4", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_map_node_nested_nodes3)
{
    auto str = R"(<root a="0"><a x="1">2</a></root>)";
    // { "-a": "0", "a": { "#text": "2", "-x": "1" } }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("-a");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(0, val.Int64());
    val = node->At("a");
    EXPECT_FALSE(val.IsEmpty());
    auto a_node = val.QueryPtr<INode>();
    val         = a_node->At("-x");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(1, val.Int64());
    val = a_node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(2, val.Int64());
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_unicode)
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
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("alphabet алфавит 字母 ← ⇐🙂", val.String());
    val = node->At("-attr");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("alphabet алфавит 字母 ← ⇐", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_unicode2)
{
    auto str = R"(<root>alphabet <p>алфавит</p> 字母 ← ⇐&#x1f642;</root>)";
    //{ "root": ["alphabet ", {"p" : "алфавит" }, " 字母 ← ⇐🙂"]}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("root");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    val           = arr_node->At(2);
    EXPECT_EQ(" 字母 ← ⇐🙂", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_unicode3)
{
    auto str = R"(<?xml version="1.0" encoding="utf-8"?><root>&#x13003; &#x2192; &#8660; &#x1f642;</root>)";
    //{ "root": "→ ⇔ 🙂" }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("root");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ("𓀃 → ⇔ 🙂", val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_empty_tags1)
{
    auto str = R"(<root><a/></root>)";
    // { "a": {}}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("a");
    EXPECT_FALSE(val.IsEmpty());
    auto a_node = val.QueryPtr<INode>();
    EXPECT_TRUE(a_node);
    EXPECT_EQ(0, a_node->Size());
    EXPECT_TRUE(a_node->IsName("a"));
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_empty_tags2)
{
    auto str = R"(<root><arr>1</arr><arr/><arr>2</arr></root>)";
    // { "arr" : [ "1", {}, "2" ]}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    val = arr_node->At(1);
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node2 = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node2);
    EXPECT_EQ(0, arr_node2->Size());
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_empty_tags3)
{
    auto str = R"(<root><c>some<br/>text</c></root>)";
    // { "c": { "#text": [ "some", {}, "text" ]} }
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("c");
    EXPECT_FALSE(val.IsEmpty());
    auto inn_node = val.QueryPtr<INode>();
    val           = inn_node->At(kXMLValueName);
    EXPECT_FALSE(val.IsEmpty());
    auto c_node = val.QueryPtr<INode>();
    EXPECT_TRUE(c_node);
    val = c_node->At(1);
    EXPECT_FALSE(val.IsEmpty());
    auto br_node = val.QueryPtr<INode>();
    EXPECT_TRUE(br_node);
    EXPECT_EQ(0, br_node->Size());
    EXPECT_TRUE(br_node->IsName("br"));
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_empty_tags4)
{
    auto str = R"(<root><arr>1</arr><empty/><arr>2</arr></root>)";
    // { "arr" : [ "1", {}, "2" ]}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("arr");
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node);
    val = arr_node->At(1);
    EXPECT_FALSE(val.IsEmpty());
    auto arr_node2 = val.QueryPtr<INode>();
    EXPECT_TRUE(arr_node2);
    EXPECT_EQ(0, arr_node2->Size());
    EXPECT_TRUE(arr_node2->IsName("empty"));
}

TEST(xnode_xml_import_unit_tests, check_valid_node_with_long_text)
{
    std::string long_text = R"(
ASCII art is a graphic design technique that uses computers for presentation 
and consists of pictures pieced together from the 95 printable (from a total of 128) 
characters defined by the ASCII Standard from 1963 and ASCII compliant character
 sets with proprietary extended characters (beyond the 128 characters of standard 7-bit ASCII). 
The term is also loosely used to refer to text-based visual art in general. 
ASCII art can be created with any text editor, and is often used with free-form languages. 
Most examples of ASCII art require a fixed-width font (non-proportional fonts,
 as on a traditional typewriter) such as Courier for presentation.

Among the oldest known examples of ASCII art are the creations by computer-art 
pioneer Kenneth Knowlton from around 1966, who was working for Bell Labs at the time.[1] 
"Studies in Perception I" by Knowlton and Leon Harmon from 1966 shows some examples of their early ASCII art.[2]

ASCII art was invented, in large part, because early printers often lacked graphics 
ability and thus, characters were used in place of graphic marks. Also, to mark divisions 
between different print jobs from different users, bulk printers often used ASCII art 
to print large banner pages, making the division easier to spot so that the results 
could be more easily separated by a computer operator or clerk.[3] 
ASCII art was also used in early e-mail when images could not be embedded.

  (\_/)
 (='.'=)
 (")_(")
)";
    auto        str       = "<root>" + long_text + "</root>";
    // { "root" : "`song text is here`"}
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("root"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("root");
    EXPECT_FALSE(val.IsEmpty());
    EXPECT_EQ(long_text, val.String());
}

TEST(xnode_xml_import_unit_tests, check_valid_big_xml)
{
    auto str           = R"(<?xml version="1.0"?>
<catalog>
   <book id="bk101">
      <author>Gambardella, Matthew</author>
      <title>XML Developer's Guide</title>
      <genre>Computer</genre>
      <price>44.95</price>
      <publish_date>2000-10-01</publish_date>
      <description>An in-depth look at creating applications 
      with XML.</description>
   </book>
   <book id="bk102">
      <author>Ralls, Kim</author>
      <title>Midnight Rain</title>
      <genre>Fantasy</genre>
      <price>5.95</price>
      <publish_date>2000-12-16</publish_date>
      <description>A former architect battles corporate zombies, 
      an evil sorceress, and her own childhood to become queen 
      of the world.</description>
   </book>
   <book id="bk103">
      <author>Corets, Eva</author>
      <title>Maeve Ascendant</title>
      <genre>Fantasy</genre>
      <price>5.95</price>
      <publish_date>2000-11-17</publish_date>
      <description>After the collapse of a nanotechnology 
      society in England, the young survivors lay the 
      foundation for a new society.</description>
   </book>
   <book id="bk104">
      <author>Corets, Eva</author>
      <title>Oberon's Legacy</title>
      <genre>Fantasy</genre>
      <price>5.95</price>
      <publish_date>2001-03-10</publish_date>
      <description>In post-apocalypse England, the mysterious 
      agent known only as Oberon helps to create a new life 
      for the inhabitants of London. Sequel to Maeve 
      Ascendant.</description>
   </book>
   <book id="bk105">
      <author>Corets, Eva</author>
      <title>The Sundered Grail</title>
      <genre>Fantasy</genre>
      <price>5.95</price>
      <publish_date>2001-09-10</publish_date>
      <description>The two daughters of Maeve, half-sisters, 
      battle one another for control of England. Sequel to 
      Oberon's Legacy.</description>
   </book>
   <book id="bk106">
      <author>Randall, Cynthia</author>
      <title>Lover Birds</title>
      <genre>Romance</genre>
      <price>4.95</price>
      <publish_date>2000-09-02</publish_date>
      <description>When Carla meets Paul at an ornithology 
      conference, tempers fly as feathers get ruffled.</description>
   </book>
   <book id="bk107">
      <author>Thurman, Paula</author>
      <title>Splish Splash</title>
      <genre>Romance</genre>
      <price>4.95</price>
      <publish_date>2000-11-02</publish_date>
      <description>A deep sea diver finds true love twenty 
      thousand leagues beneath the sea.</description>
   </book>
   <book id="bk108">
      <author>Knorr, Stefan</author>
      <title>Creepy Crawlies</title>
      <genre>Horror</genre>
      <price>4.95</price>
      <publish_date>2000-12-06</publish_date>
      <description>An anthology of horror stories about roaches,
      centipedes, scorpions  and other insects.</description>
   </book>
   <book id="bk109">
      <author>Kress, Peter</author>
      <title>Paradox Lost</title>
      <genre>Science Fiction</genre>
      <price>6.95</price>
      <publish_date>2000-11-02</publish_date>
      <description>After an inadvertant trip through a Heisenberg
      Uncertainty Device, James Salway discovers the problems 
      of being quantum.</description>
   </book>
   <book id="bk110">
      <author>O'Brien, Tim</author>
      <title>Microsoft .NET: The Programming Bible</title>
      <genre>Computer</genre>
      <price>36.95</price>
      <publish_date>2000-12-09</publish_date>
      <description>Microsoft's .NET initiative is explored in 
      detail in this deep programmer's reference.</description>
   </book>
   <book id="bk111">
      <author>O'Brien, Tim</author>
      <title>MSXML3: A Comprehensive Guide</title>
      <genre>Computer</genre>
      <price>36.95</price>
      <publish_date>2000-12-01</publish_date>
      <description>The Microsoft MSXML3 parser is covered in 
      detail, with attention to XML DOM interfaces, XSLT processing, 
      SAX and more.</description>
   </book>
   <book id="bk112">
      <author>Galos, Mike</author>
      <title>Visual Studio 7: A Comprehensive Guide</title>
      <genre>Computer</genre>
      <price>49.95</price>
      <publish_date>2001-04-16</publish_date>
      <description>Microsoft Visual Studio 7 is explored in depth,
      looking at how Visual Basic, Visual C++, C#, and ASP+ are 
      integrated into a comprehensive development 
      environment.</description>
   </book>
</catalog>)";
    auto [node, e_pos] = xnode::FromXml(str);
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(0, e_pos);
#ifdef _DEBUG
    auto json = xnode::ToJson(node);
#endif
    EXPECT_TRUE(node->IsName("catalog"));
    EXPECT_EQ(INode::NodeType::Map, node->Type());
    auto val = node->At("book");
    EXPECT_FALSE(val.IsEmpty());
    auto book_node = val.QueryPtr<INode>();
    EXPECT_EQ(INode::NodeType::Array, book_node->Type());
    EXPECT_EQ(12, book_node->Size());
}

// NOLINTEND(*)
