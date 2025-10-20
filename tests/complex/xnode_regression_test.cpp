#include "xnode.h"
#include "xnode_factory.h"
#include "xnode_functions.h"
#include "xnode_json.h"

// For XNode::Counter()
#include "../src/xnode/impl/xnode_impl.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_regression_tests, map_set_to_exists_key)
{
    auto parent_node = xnode::Create(INode::NodeType::Map, "parent");
    auto child1      = xnode::Create(INode::NodeType::Map, "child");
    auto res         = parent_node->Set(child1->NameGet(), child1);
    EXPECT_TRUE(res.first);
    auto val = parent_node->At("child");
    EXPECT_TRUE(val);
    auto child1_res = val.QueryPtr<INode>();
    EXPECT_TRUE(child1_res);
    EXPECT_TRUE(child1_res->IsName("child"));

    auto child2 = xnode::Create(INode::NodeType::Array, "child_arr");
    child2->Insert(xnode::kIdxBegin, "some_value");
    res = parent_node->Set(child1->NameGet(), child2);
    EXPECT_TRUE(res.first);
    val = parent_node->At("child");
    EXPECT_TRUE(val);
    auto child2_res = val.QueryPtr<INode>();
    EXPECT_TRUE(child2_res);
    EXPECT_TRUE(child2_res->IsName("child"));
    EXPECT_EQ(1, child2_res->Size());
    EXPECT_EQ("some_value", child2_res->At(xnode::kIdxBegin).String());
}

#ifndef _DEBUG
TEST(xnode_regression_tests, node_parent_circular_set_memleak_in_cb)
{
    auto node_map_sp  = xnode::Create(INode::NodeType::Map, "root");
    auto node_map_sp2 = xnode::CreateMap({{"name2", "02"}}, "02");

    auto [ok, prev] = node_map_sp2->ParentSet(node_map_sp);
    EXPECT_TRUE(ok);

    node_map_sp2->Set("place_for_node", 123);
    node_map_sp2->ForEach([&](const auto& key, auto& val) {
        if (key == XKey("place_for_node")) {
            val = XValue(node_map_sp);
            return xnode::OnEachRes::Stop;
        }
        return xnode::OnEachRes::Next;
    });
}
#endif

// NOLINTEND(*)