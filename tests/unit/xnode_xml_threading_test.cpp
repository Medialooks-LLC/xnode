#include "xnode.h"
#include "xnode_xml.h"

#ifdef _DEBUG
    #include "xnode_json.h"
#endif

#include <gtest/gtest.h>
#include <thread>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_xml_threaded_tests, check_valid_map_node_nested_node_in_text)
{
    auto test_init = []() {
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
    };

    std::vector<std::thread> test_threads;
    for (int i = 1; i <= 1100; i++) {
        test_threads.emplace_back(test_init);
    }
    for (auto& thread : test_threads) {
        thread.join();
    }
}

// NOLINTEND(*)
