#include "xnode.h"
#include "xnode_factory.h"
#include "xnode_functions.h"
#include "xnode_json.h"

// For XNode::counter()
#include "../src/xnode/impl/xnode_impl.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_utests, map_check_type)
{
    auto map_node = xnode::Create(INode::NodeType::Map);

    EXPECT_EQ(map_node->Type(), INode::NodeType::Map);
}

TEST(xnode_utests, array_check_type)
{
    auto arr_node = xnode::Create(INode::NodeType::Array);

    EXPECT_EQ(arr_node->Type(), INode::NodeType::Array);
}

TEST(xnode_utests, map_check_name_get)
{
    auto map_node_without_name = xnode::Create(INode::NodeType::Map);

    EXPECT_EQ(map_node_without_name->NameGet(), std::string());

    auto map_node_with_name = xnode::Create(INode::NodeType::Map, "some name");
    EXPECT_EQ(map_node_with_name->NameGet(), std::string("some name"));
}

TEST(xnode_utests, array_check_name_get)
{
    auto arr_node_without_name = xnode::Create(INode::NodeType::Array);

    EXPECT_EQ(arr_node_without_name->NameGet(), std::string());

    auto arr_node_with_name = xnode::Create(INode::NodeType::Array, "Some another name");
    EXPECT_EQ(arr_node_with_name->NameGet(), std::string("Some another name"));
}

TEST(xnode_utests, map_check_name_set)
{
    auto map_node = xnode::Create(INode::NodeType::Map);

    EXPECT_EQ(map_node->NameGet(), std::string());

    // Check single change
    map_node->NameSet("some name", false);
    EXPECT_EQ(map_node->NameGet(), std::string("some name"));

    // Check second change
    map_node->NameSet("some another name", false);
    EXPECT_EQ(map_node->NameGet(), std::string("some another name"));
}

TEST(xnode_utests, array_check_name_set)
{
    auto arr_node = xnode::Create(INode::NodeType::Array);

    EXPECT_EQ(arr_node->NameGet(), std::string());

    // Check single change
    arr_node->NameSet("some name", false);
    EXPECT_EQ(arr_node->NameGet(), std::string("some name"));

    // Check second change
    arr_node->NameSet("some another name", false);
    EXPECT_EQ(arr_node->NameGet(), std::string("some another name"));
}

TEST(xnode_utests, map_check_is_name)
{
    auto map_node = xnode::Create(INode::NodeType::Map);

    EXPECT_TRUE(map_node->IsName(""));
    EXPECT_FALSE(map_node->IsName("bla-bla-bla"));

    map_node->NameSet("some name", false);
    EXPECT_FALSE(map_node->IsName(""));
    EXPECT_FALSE(map_node->IsName("bla-bla-bla"));
    EXPECT_TRUE(map_node->IsName("some name"));
}

TEST(xnode_utests, array_check_is_name)
{
    auto arr_node = xnode::Create(INode::NodeType::Array);

    EXPECT_TRUE(arr_node->IsName(""));
    EXPECT_FALSE(arr_node->IsName("bla-bla-bla"));

    arr_node->NameSet("some name", false);
    EXPECT_FALSE(arr_node->IsName(""));
    EXPECT_FALSE(arr_node->IsName("bla-bla-bla"));
    EXPECT_TRUE(arr_node->IsName("some name"));
}

TEST(xnode_utests, map_check_parent_get_set_invalid_parent)
{
    auto check_set_invalid_parent = [](INode::SPtr child, INode::SPtr parent, std::string name = "") {
        std::pair<bool, INode::SPtr> res;
        if (name == "") {
            res = child->ParentSet(parent);
        }
        else {
            res = child->ParentSet(parent, name);
        }
        EXPECT_FALSE(res.first);
        EXPECT_TRUE(res.second == nullptr);
        auto current_parent = child->ParentGet();
        EXPECT_TRUE(current_parent == nullptr);
    };

    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");

    INode::SPtr parent = nullptr;
    check_set_invalid_parent(map_node, parent);
    check_set_invalid_parent(map_node, parent, "Try to use some name");
}

TEST(xnode_utests, array_check_parent_get_set_invalid_parent)
{
    auto check_set_invalid_parent = [](INode::SPtr child, INode::SPtr parent, std::string name = "") {
        std::pair<bool, INode::SPtr> res;
        if (name == "") {
            res = child->ParentSet(parent);
        }
        else {
            res = child->ParentSet(parent, name);
        }
        EXPECT_FALSE(res.first);
        EXPECT_TRUE(res.second == nullptr);
        auto current_parent = child->ParentGet();
        EXPECT_TRUE(current_parent == nullptr);
    };

    auto arr_node = xnode::Create(INode::NodeType::Array, "childArray");

    INode::SPtr parent = nullptr;
    check_set_invalid_parent(arr_node, parent);
    check_set_invalid_parent(arr_node, parent, "Try to use some name");
}

TEST(xnode_utests, map_check_parent_get_set_invalid_parent_with_valid_prev_parent)
{
    auto check_set_invalid_parent = [](INode::SPtr child, INode::SPtr parent, std::string name = "") {
        std::pair<bool, INode::SPtr> res;
        if (name == "") {
            res = child->ParentSet(parent);
        }
        else {
            res = child->ParentSet(parent, name);
        }
        EXPECT_TRUE(res.first);
        EXPECT_TRUE(res.second != nullptr);
        auto current_parent = child->ParentGet();
        EXPECT_TRUE(current_parent == nullptr);
    };

    auto good_parent     = xnode::Create(INode::NodeType::Map, "parentMap");
    auto set_good_parent = [&good_parent](INode::SPtr child) {
        auto [ok, _] = child->ParentSet(good_parent);
        EXPECT_TRUE(ok);
    };

    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");

    set_good_parent(map_node);
    INode::SPtr parent = nullptr;
    check_set_invalid_parent(map_node, parent);
    set_good_parent(map_node);
    check_set_invalid_parent(map_node, parent, "Try to use some name");
}

TEST(xnode_utests, array_check_parent_get_set_invalid_parent_with_valid_prev_parent)
{
    auto check_set_invalid_parent = [](INode::SPtr child, INode::SPtr parent, std::string name = "") {
        std::pair<bool, INode::SPtr> res;
        if (name == "") {
            res = child->ParentSet(parent);
        }
        else {
            res = child->ParentSet(parent, name);
        }
        EXPECT_TRUE(res.first);
        EXPECT_TRUE(res.second != nullptr);
        auto current_parent = child->ParentGet();
        EXPECT_TRUE(current_parent == nullptr);
    };

    auto good_parent     = xnode::Create(INode::NodeType::Map, "parentMap");
    auto set_good_parent = [&good_parent](INode::SPtr child) {
        auto [ok, _] = child->ParentSet(good_parent);
        EXPECT_TRUE(ok);
    };

    auto arr_node = xnode::Create(INode::NodeType::Array, "childArray");

    set_good_parent(arr_node);
    INode::SPtr parent = nullptr;
    check_set_invalid_parent(arr_node, parent);
    set_good_parent(arr_node);
    check_set_invalid_parent(arr_node, parent, "Try to use some name");
}

TEST(xnode_utests, map_check_parent_get_set_valid_parent)
{
    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok1, prevP1]  = map_node->ParentSet(new_parent_map, new_parent_map->NameGet());
    EXPECT_TRUE(ok1);
    EXPECT_TRUE(prevP1 == nullptr);
    auto parent = map_node->ParentGet();
    EXPECT_EQ(parent, new_parent_map);
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok2, prevP2]  = map_node->ParentSet(new_parent_arr, new_parent_arr->NameGet());
    EXPECT_TRUE(ok2);
    EXPECT_TRUE(prevP2 != nullptr);
    parent = map_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr);
}

TEST(xnode_utests, array_check_parent_get_set_valid_parent)
{
    auto arr_node = xnode::Create(INode::NodeType::Array, "childArray");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok3, prevP3]  = arr_node->ParentSet(new_parent_map, new_parent_map->NameGet());
    EXPECT_TRUE(ok3);
    EXPECT_TRUE(prevP3 == nullptr);
    auto parent = arr_node->ParentGet();
    EXPECT_EQ(parent, new_parent_map);
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok4, prevP4]  = arr_node->ParentSet(new_parent_arr, new_parent_arr->NameGet());
    EXPECT_TRUE(ok4);
    EXPECT_TRUE(prevP4 != nullptr);
    parent = arr_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr);
}

TEST(xnode_utests, map_check_parent_get_set_with_empty_name_parameter)
{
    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok1, prevP1]  = map_node->ParentSet(new_parent_map);
    EXPECT_TRUE(ok1);
    EXPECT_TRUE(prevP1 == nullptr);
    auto parent = map_node->ParentGet();
    EXPECT_EQ(parent, new_parent_map);
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok2, prevP2]  = map_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok2);
    EXPECT_TRUE(prevP2 != nullptr);
    parent = map_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr);
}

TEST(xnode_utests, array_check_parent_get_set_with_empty_name_parameter)
{
    auto arr_node = xnode::Create(INode::NodeType::Array, "childArray");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok3, prevP3]  = arr_node->ParentSet(new_parent_map);
    EXPECT_TRUE(ok3);
    EXPECT_TRUE(prevP3 == nullptr);
    auto parent = arr_node->ParentGet();
    EXPECT_EQ(parent, new_parent_map);
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok4, prevP4]  = arr_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok4);
    EXPECT_TRUE(prevP4 != nullptr);
    parent = arr_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr);
}

TEST(xnode_utests, map_check_parent_get_set_with_unnamed_child)
{
    auto map_node = xnode::Create(INode::NodeType::Map);

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok1, prevP1]  = map_node->ParentSet(new_parent_map);
    EXPECT_FALSE(ok1);
    EXPECT_TRUE(prevP1 == nullptr);
    auto parent = map_node->ParentGet();
    EXPECT_TRUE(parent == nullptr); // If parent is a map we must set a name for the child
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok2, prevP2]  = map_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok2);
    EXPECT_TRUE(prevP2 == nullptr);
    parent = map_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr); // If the parent is an Array, we do not have to give a name for the child
}

TEST(xnode_utests, array_check_parent_get_set_with_unnamed_child)
{
    auto arr_node = xnode::Create(INode::NodeType::Array);

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok3, prevP3]  = arr_node->ParentSet(new_parent_map);
    EXPECT_FALSE(ok3);
    EXPECT_TRUE(prevP3 == nullptr);
    auto parent = arr_node->ParentGet();
    EXPECT_TRUE(parent == nullptr); // If parent is a map we must set a name for the child
    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok4, prevP4]  = arr_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok4);
    EXPECT_TRUE(prevP4 == nullptr);
    parent = arr_node->ParentGet();
    EXPECT_EQ(parent, new_parent_arr); // If the parent is an Array, we do not have to give a name for the child
}

TEST(xnode_utests, map_check_parent_detach_when_no_parent)
{
    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");
    auto parent   = map_node->ParentDetach();
    EXPECT_TRUE(parent == nullptr);
}

TEST(xnode_utests, array_check_parent_detach_when_no_parent)
{
    auto arr_node = xnode::Create(INode::NodeType::Array, "childArray");
    auto parent   = arr_node->ParentDetach();
    EXPECT_TRUE(parent == nullptr);
}

TEST(xnode_utests, map_check_parent_detach)
{
    auto map_node = xnode::Create(INode::NodeType::Map, "childMap");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok1, prevP1]  = map_node->ParentSet(new_parent_map);
    EXPECT_TRUE(ok1);
    auto parent = map_node->ParentDetach();
    EXPECT_EQ(parent, new_parent_map);
    parent = map_node->ParentGet();
    EXPECT_TRUE(parent == nullptr);

    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok2, prevP2]  = map_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok2);
    parent = map_node->ParentDetach();
    EXPECT_EQ(parent, new_parent_arr);
    parent = map_node->ParentGet();
    EXPECT_TRUE(parent == nullptr);
}

TEST(xnode_utests, array_check_parent_detach)
{
    auto arr_node = xnode::Create(INode::NodeType::Array, "childArr");

    auto new_parent_map = xnode::Create(INode::NodeType::Map, "parentMap");
    auto [ok3, prevP3]  = arr_node->ParentSet(new_parent_map);
    EXPECT_TRUE(ok3);
    auto parent = arr_node->ParentDetach();
    EXPECT_EQ(parent, new_parent_map);
    parent = arr_node->ParentGet();
    EXPECT_TRUE(parent == nullptr);

    auto new_parent_arr = xnode::Create(INode::NodeType::Array, "parentArr");
    auto [ok4, prevP4]  = arr_node->ParentSet(new_parent_arr);
    EXPECT_TRUE(ok4);
    parent = arr_node->ParentDetach();
    EXPECT_EQ(parent, new_parent_arr);
    parent = arr_node->ParentGet();
    EXPECT_TRUE(parent == nullptr);
}

TEST(xnode_utests, emplace_to_array)
{
    auto node = xnode::Create(INode::NodeType::Map);
    xnode::EmplaceToArray(node, "test_array", 12);
    auto array_node = xnode::NodeGet(node, "test_array");
    ASSERT_TRUE(array_node) << "array node not created";
    EXPECT_EQ(array_node->Type(), INode::NodeType::Array) << "array node have wrong type";
    EXPECT_EQ(array_node->Size(), 1) << "array node have wrong size (not one element)";
    xnode::EmplaceToArray(node, "test_array", "next_value");
    EXPECT_EQ(array_node->Size(), 2) << "array node have wrong size (not two element)";
    EXPECT_EQ(array_node->At(0), 12) << "wrong array[0] value";
    EXPECT_EQ(array_node->At(1), "next_value") << "wrong array[1] value";
}

TEST(xnode_utests, emplace_to_array_w_convert)
{
    auto node = xnode::Create(INode::NodeType::Map);
    node->Set("test_array", "initial value");

    xnode::EmplaceToArray(node, "test_array", 12);
    auto array_node = xnode::NodeGet(node, "test_array");
    ASSERT_TRUE(array_node) << "array node not created";
    EXPECT_EQ(array_node->Type(), INode::NodeType::Array) << "array node have wrong type";
    EXPECT_EQ(array_node->Size(), 2) << "array node have wrong size (not two elements)";
    xnode::EmplaceToArray(node, "test_array", "next_value");
    EXPECT_EQ(array_node->Size(), 3) << "array node have wrong size (not three elements)";
    EXPECT_EQ(array_node->At(0), "initial value") << "wrong array[0] value";
    EXPECT_EQ(array_node->At(1), 12) << "wrong array[1] value";
    EXPECT_EQ(array_node->At(2), "next_value") << "wrong array[2] value";
}


// NOLINTEND(*)