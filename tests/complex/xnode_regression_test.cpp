#include "xnode.h"
#include "xnode_factory.h"
#include "xnode_functions.h"
#include "xnode_json.h"

// For XNode::counter()
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
    auto res = parent_node->Set(child1->NameGet(), child1);
    EXPECT_TRUE(res.first);
    auto val = parent_node->At("child");
    EXPECT_TRUE(val);
    auto child1_res = val.QueryPtr<INode>();
    EXPECT_TRUE(child1_res);
    EXPECT_TRUE(child1_res->IsName("child"));

    auto child2      = xnode::Create(INode::NodeType::Array, "child_arr");
    child2->Insert(kIdxBegin, "some_value");
    res = parent_node->Set(child1->NameGet(), child2);
    EXPECT_TRUE(res.first);
    val = parent_node->At("child");
    EXPECT_TRUE(val);
    auto child2_res = val.QueryPtr<INode>();
    EXPECT_TRUE(child2_res);
    EXPECT_TRUE(child2_res->IsName("child"));
    EXPECT_EQ(1, child2_res->Size());
    EXPECT_EQ("some_value", child2_res->At(kIdxBegin).String());
}

// NOLINTEND(*)