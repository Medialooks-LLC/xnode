#include "xnode.h"
#include "xnode_functions.h"
#include "xnode_json.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_bulk_tests, BulkSet)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);

    std::vector<std::pair<XKey, XValue>> vec_set = {{"dbl", 123.1}, {"str", "string"}, {"int", 77}, {"bool", true}};

    auto vec_copy = vec_set;
    auto count    = node_map_sp->BulkSet(std::move(vec_set));
    EXPECT_EQ(count, vec_copy.size());
    EXPECT_EQ(vec_set.size(), 0);
    EXPECT_EQ(node_map_sp->At("str"), "string");
    EXPECT_EQ(node_map_sp->At("dbl"), 123.1);
    EXPECT_EQ(node_map_sp->At("int"), 77);
    EXPECT_EQ(node_map_sp->At("bool"), true);

    auto vec_all = vec_copy;
    count        = node_map_sp->BulkSet(std::move(vec_copy));
    EXPECT_EQ(count, 0);
    EXPECT_EQ(vec_copy.size(), vec_all.size());

    auto removed = node_map_sp->BulkErase({"str", "int"});
    EXPECT_EQ(removed.size(), 2);

    count = node_map_sp->BulkSet(std::move(vec_copy));
    EXPECT_EQ(count, 2);
    EXPECT_EQ(vec_copy.size() + count, vec_all.size());

    EXPECT_EQ(vec_copy[0].first, XKey("dbl"));
    EXPECT_EQ(vec_copy[1].first, XKey("bool"));

    count = node_map_sp->BulkSet({{"new", 177}, {"int", 12}});
    EXPECT_EQ(count, 2);
    EXPECT_EQ(node_map_sp->Size(), vec_all.size() + 1);

    EXPECT_EQ(node_map_sp->At("int"), 12);
    EXPECT_EQ(node_map_sp->At("new"), 177);
}
TEST(xnode_bulk_tests, bulk_set_node)
{
    auto node_map_sp   = xnode::Create(INode::NodeType::Map);
    auto node_array_sp = xnode::CreateArray({});

    auto node_map_sp1 = xnode::CreateMap({{"name1", "01"}}, "01");
    auto node_map_sp2 = xnode::CreateMap({{"name2", "02"}}, "02");
    auto node_map_sp3 = xnode::CreateMap({{"name3", "03"}}, "03");

    node_map_sp1->ParentSet(node_map_sp);
    node_map_sp2->ParentSet(node_map_sp);
    node_map_sp3->ParentSet(node_map_sp);

    EXPECT_EQ(node_map_sp->Size(), 3);

    // After Insert into array, nodes should be removed from map
    auto [count,
          last_key] = node_array_sp->BulkInsert(0, {node_map_sp1, "a234", node_map_sp2, true, nullptr, node_map_sp3});
    EXPECT_EQ(count, 6);
    EXPECT_EQ(last_key.IndexGet().value_or(-1), 5);

    auto arr_json = xnode::ToJson(node_array_sp);

    EXPECT_EQ(node_map_sp->Size(), 0);
    EXPECT_EQ(node_array_sp->At(0), node_map_sp1);
    EXPECT_EQ(node_array_sp->At(1), "a234");
    EXPECT_EQ(node_array_sp->At(2), node_map_sp2);
    EXPECT_EQ(node_array_sp->At(3), true);
    EXPECT_EQ(node_array_sp->At(4).Type(), XValue::kNull);
    EXPECT_EQ(node_array_sp->At(5), node_map_sp3);

    // No insertion nodes what are already in array
    std::tie(count, last_key) = node_array_sp->BulkInsert(kIdxLast, {node_map_sp1, 999.9, node_map_sp2, node_map_sp3});
    EXPECT_EQ(count, 1);
    EXPECT_EQ(node_array_sp->At(5), 999.9);
    EXPECT_EQ(node_array_sp->At(6), node_map_sp3);
    EXPECT_EQ(last_key.IndexGet().value_or(-1), 5);

    count    = node_map_sp->BulkInsert({{"001", node_map_sp1}, {"002", node_map_sp1}, {"003", node_map_sp1}});
    arr_json = xnode::ToJson(node_map_sp);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(node_map_sp->Size(), 1);
    EXPECT_EQ(node_array_sp->Size(), 6);
    EXPECT_EQ(node_map_sp->At("001"), node_map_sp1);

    count    = node_map_sp->BulkInsert({{"001a", node_map_sp1}, {"002", node_map_sp2}, {"003", node_map_sp3}});
    arr_json = xnode::ToJson(node_map_sp);

    EXPECT_EQ(count, 3);
    EXPECT_EQ(node_map_sp->Size(), 3);
    EXPECT_EQ(node_array_sp->Size(), 4);

    EXPECT_EQ(node_map_sp->At("001a"), node_map_sp1);
    EXPECT_EQ(node_map_sp->At("002"), node_map_sp2);
    EXPECT_EQ(node_map_sp->At("003"), node_map_sp3);

    EXPECT_EQ(node_map_sp1->NameGet(), "001a");
    EXPECT_EQ(node_map_sp2->NameGet(), "002");
    EXPECT_EQ(node_map_sp3->NameGet(), "003");
}

TEST(xnode_bulk_tests, bulk_set_nodes_circular)
{
// Failed in debug in assert for circular
#ifndef _DEBUG
    auto node_map_sp   = xnode::CreateMap({{"root", true}}, "root");
    auto node_array_sp = xnode::CreateArray({}, "array");

    auto node_map_sp1 = xnode::CreateMap({{"name1", "01"}}, "01");
    auto node_map_sp2 = xnode::CreateMap({{"name2", "02"}}, "02");
    auto node_map_sp3 = xnode::CreateMap({{"name3", "03"}}, "03");

    node_map_sp1->ParentSet(node_map_sp);
    node_map_sp2->ParentSet(node_map_sp1);
    node_map_sp3->ParentSet(node_map_sp2);
    node_map_sp->ParentSet(node_array_sp);

    EXPECT_EQ(node_map_sp->Size(), 2);

    auto count = node_map_sp3->BulkInsert(
        {{"001", node_array_sp}, {"zzz", node_map_sp}, {"002", node_map_sp1}, {"003", node_map_sp2}});

    auto arr_json = xnode::ToJson(node_array_sp);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(node_map_sp3->Size(), 1);

    count = node_map_sp3->BulkSet(
        {{"001", node_array_sp}, {"zzz", node_map_sp}, {"002", node_map_sp1}, {"003", node_map_sp2}});

    arr_json = xnode::ToJson(node_array_sp);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(node_map_sp3->Size(), 1);

    count = node_array_sp->BulkInsert(0, {node_map_sp, node_map_sp1, node_map_sp2, node_map_sp3}).first;
    EXPECT_EQ(count, 3);

    arr_json = xnode::ToJson(node_array_sp);

    auto ok = node_array_sp->ParentSet(node_map_sp).first;
    EXPECT_FALSE(ok);

    node_map_sp->ParentDetach();
    ok = node_array_sp->ParentSet(node_map_sp).first;
    EXPECT_TRUE(ok);

    arr_json = xnode::ToJson(node_map_sp);

    node_map_sp1->ParentDetach();
    ok = node_map_sp->ParentSet(node_map_sp1).first;
    EXPECT_TRUE(ok);

    arr_json = xnode::ToJson(node_map_sp1);

    node_map_sp2->ParentDetach();
    ok = node_map_sp1->ParentSet(node_map_sp2).first;
    EXPECT_TRUE(ok);

    ok = node_map_sp2->ParentSet(node_map_sp3).first;
    EXPECT_FALSE(ok);

    node_map_sp3->ParentDetach();
    ok = node_map_sp2->ParentSet(node_map_sp3).first;
    EXPECT_TRUE(ok);

    arr_json = xnode::ToJson(node_map_sp3);

    count = node_array_sp->BulkInsert(0, {node_map_sp, node_map_sp1, node_map_sp2, node_map_sp3}).first;
    EXPECT_EQ(count, 0);
#endif
}

// NOLINTEND(*)