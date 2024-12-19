#include "xnode.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_tests_json, conversion_test)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next", "value"), 999);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next2", "value"), 99.0);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next3::subnext", "zzz"), "TEST_vs1234");
    xnode::Set(node_map_sp, "node1::xxx::value", 99.0);
    xnode::Set(node_map_sp, "node2::null", nullptr);
    xnode::Set(node_map_sp, "int_val", 10);
    xnode::Set(node_map_sp, "uint_val", 10U); // todo: ULL for gcc
    auto node_array = xnode::NodeGet(node_map_sp, "node1::array", INode::NodeType::Array);
    ASSERT_TRUE(node_array);
    node_array->Insert(kIdxEnd, 123);
    node_array->Insert(kIdxEnd, "string");
    node_array->Insert(kIdxEnd, 123.456);
    node_array->Insert(kIdxEnd, nullptr);
    node_array->Insert(kIdxEnd, true);
    node_array->Insert(kIdxEnd, std::numeric_limits<uint64_t>::max()); // CHECK !!!
    node_array->Insert(kIdxEnd, std::numeric_limits<int64_t>::min());
    node_array->Insert(kIdxBegin, "first");
    node_array->Insert(1, "second");

    auto str = xnode::ToJson(node_map_sp, nullptr, xnode::JsonFormat::kOneLineArrays);
    std::cout << "ORIGINAL:" << str << std::endl;
    EXPECT_FALSE(str.empty());

    auto [node_check, err_pos] = xnode::FromJson(str);
    ASSERT_TRUE(node_check);
    str = xnode::ToJson(node_check, nullptr, xnode::JsonFormat::kOneLineArrays);
    std::cout << "CONVERTED:" << str << std::endl;

    EXPECT_EQ(err_pos, 0);
    size_t zDiff = 0;
    xnode::Compare(node_map_sp,
                   node_check,
                   true,
                   [&](const INode::SPtrC& ncp, const XKey& key, const XValueRT& left, const XValueRT& right) {
                       std::cout << "DIFF:" << key.StringGet().value_or("-") << "/idx:" << key.IndexGet().value_or(0)
                                 << " values:" << left.String() << " " << right.String() << std::endl;
                       ++zDiff;
                       return false;
                   });

    EXPECT_EQ(zDiff, 0);
}

TEST(xnode_tests_json, array_keep_nodes_names)
{
    const size_t nodes_count = 8;
    auto         array_node  = xnode::CreateArray();
    ASSERT_TRUE(array_node);
    for (size_t z = 0; z < nodes_count; ++z) {
        auto map_node = xnode::CreateMap({{"dbl", 10.0}, {"int", 11 + z}}, "test_node_" + std::to_string(z));
        if (z % 2)
            xnode::Set(map_node, {"subnode::subnode2", "val"}, z + 100);

        ASSERT_TRUE(map_node);
        auto res = array_node->Insert({}, map_node);
        EXPECT_TRUE(res.succeeded);
        ASSERT_TRUE(res.inserted_at.IndexGet().has_value());
        EXPECT_EQ(res.inserted_at.IndexGet().value() + 1, array_node->Size());
    }
    EXPECT_EQ(nodes_count, array_node->Size());

    // Add elements to array
    for (size_t z = 0; z < nodes_count / 2; ++z) {
        array_node->Insert(3 + z * 2, "str_" + std::to_string(z));
        array_node->Insert(1 + z * 3, z);
    }

    // In this case we lost nodes names
    auto json_def = xnode::ToJson(array_node);
    std::cout << json_def;

    // Now json contain array with values
    auto json_w_names = xnode::ToJson(xnode::ArrayNodesWrap(array_node).QueryPtrC<INode>());
    std::cout << json_w_names;
    EXPECT_NE(json_def, json_w_names);

    auto [restore_w_names, size] = xnode::FromJson(json_w_names);
    auto json_def_2              = xnode::ToJson(restore_w_names);

    auto restore_node = xnode::ArrayNodesUnwrap(restore_w_names).QueryPtrC<INode>();
    int  cmp          = xnode::Compare(array_node, restore_node, true);
    EXPECT_EQ(cmp, 0) << "xnode::Compare(array_node, restore_node) NOT EQUAL" << cmp;

    auto src_nodes   = xnode::ChildNodesConstGet(array_node);
    auto check_nodes = xnode::ChildNodesConstGet(restore_node);
    ASSERT_EQ(src_nodes.size(), check_nodes.size()) << "NOT ALL NODES RESTORED";
    for (size_t z = 0; z < src_nodes.size(); ++z) {
        ASSERT_TRUE(src_nodes[z]);
        ASSERT_TRUE(check_nodes[z]);
        EXPECT_EQ(src_nodes[z]->NameGet(), check_nodes[z]->NameGet());
        EXPECT_EQ(xnode::ToJson(src_nodes[z]), xnode::ToJson(check_nodes[z]));
    }
}

// NOLINTEND(*)
