#include "xnode.h"
#include "xnode_factory.h"
#include "xnode_functions.h"
#include "xnode_json.h"

// For XNode::Counter()
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

TEST(xnode_utests, node_get)
{
    auto root_map   = xnode::Create(INode::NodeType::Map);
    auto node_empty = xnode::NodeGet(root_map, "not_create");
    EXPECT_FALSE(node_empty) << "xnode::NodeGet() create node for empty type";

    auto node_map = xnode::NodeGet(root_map, "create_map", INode::NodeType::Map);
    EXPECT_TRUE(node_map) << "xnode::NodeGet(INode::NodeType::Map) node not created";
    EXPECT_EQ(node_map->Type(), INode::NodeType::Map) << "xnode::NodeGet(INode::NodeType::Map) wrong type";

    auto node_arr = xnode::NodeGet(root_map, "create_array", INode::NodeType::Array);
    EXPECT_TRUE(node_arr) << "xnode::NodeGet(INode::NodeType::Array) node not created";
    EXPECT_EQ(node_arr->Type(), INode::NodeType::Array) << "xnode::NodeGet(INode::NodeType::Array) wrong type";
}

TEST(xnode_utests, node_combine_no_update)
{
    auto base_map = xnode::CreateMap({{"unchnaged", 99}, {"updated", false}});

    auto check_no_update = xnode::NodeCombine(base_map, {}, false);
    EXPECT_EQ(xnode::ToJson(base_map), xnode::ToJson(check_no_update)) << "NodeCombine() wrong with empty update";
    auto check_no_update_ow = xnode::NodeCombine(base_map, {}, true);
    EXPECT_EQ(xnode::ToJson(base_map), xnode::ToJson(check_no_update_ow)) << "NodeCombine() wrong with empty update";
}

TEST(xnode_utests, node_combine_no_base)
{
    auto base_map = xnode::CreateMap({{"unchnaged", 99}, {"updated", false}});

    auto check_no_base = xnode::NodeCombine(nullptr, {{"new_val", 1}, {"updated", true}}, false);
    ASSERT_TRUE(check_no_base) << "NodeCombine() wrong with no base";
    EXPECT_EQ(check_no_base->At("new_val").Uint32(), 1);
    EXPECT_EQ(check_no_base->At("updated").Bool(), true);

    auto check_no_base_ow = xnode::NodeCombine(nullptr, {{"new_val", 1}, {"updated", true}}, false);
    ASSERT_TRUE(check_no_base_ow) << "NodeCombine() wrong with no base";
    EXPECT_EQ(check_no_base_ow->At("updated").Bool(), true);
    EXPECT_EQ(check_no_base_ow->At("updated").Bool(), true);
}

TEST(xnode_utests, node_combine)
{
    auto base_map = xnode::CreateMap({{"unchnaged", 99}, {"updated", false}});

    auto combine = xnode::NodeCombine(base_map, {{"new_val", 1}, {"updated", true}}, false);
    ASSERT_TRUE(combine) << "NodeCombine() nullptr out";
    EXPECT_EQ(combine->At("unchnaged").Uint32(), 99);
    EXPECT_EQ(combine->At("new_val").Uint32(), 1);
    EXPECT_EQ(combine->At("updated").Bool(true), false);

    auto combine_ow = xnode::NodeCombine(base_map, {{"new_val", 1}, {"updated", true}}, true);
    ASSERT_TRUE(combine_ow) << "NodeCombine() nullptr out";
    EXPECT_EQ(combine_ow->At("unchnaged").Uint32(), 99);
    EXPECT_EQ(combine_ow->At("new_val").Uint32(), 1);
    EXPECT_EQ(combine_ow->At("updated").Bool(true), true);
}

TEST(xnode_utests, clone_map_names)
{
    auto base_map = xnode::CreateMap({{"unchnaged", 99}, {"updated", false}}, "test_name");
    EXPECT_EQ(base_map->NameGet(), "test_name");

    auto clone_w_name = xnode::Clone(base_map, true);
    EXPECT_EQ(base_map->NameGet(), clone_w_name->NameGet());

    auto clone_wo_name = xnode::Clone(base_map, true, {}, "");
    EXPECT_EQ(clone_wo_name->NameGet(), "");

    auto clone_new_name = xnode::Clone(base_map, true, {}, "new_name");
    EXPECT_EQ(clone_new_name->NameGet(), "new_name");
}

TEST(xnode_utests, clone_arr_names)
{
    auto base_arr = xnode::CreateArray({"unchnaged", 99, "updated", false}, "test_name");
    EXPECT_EQ(base_arr->NameGet(), "test_name");

    auto clone_w_name = xnode::Clone(base_arr, true);
    EXPECT_EQ(base_arr->NameGet(), clone_w_name->NameGet());

    auto clone_wo_name = xnode::Clone(base_arr, true, {}, "");
    EXPECT_EQ(clone_wo_name->NameGet(), "");

    auto clone_new_name = xnode::Clone(base_arr, true, {}, "new_name");
    EXPECT_EQ(clone_new_name->NameGet(), "new_name");
}

TEST(xnode_utests, clone_empty_map_names)
{
    auto base_map = xnode::CreateMap({}, "test_name");
    EXPECT_EQ(base_map->NameGet(), "test_name");

    auto clone_w_name = xnode::Clone(base_map, true);
    EXPECT_EQ(base_map->NameGet(), clone_w_name->NameGet());

    auto clone_wo_name = xnode::Clone(base_map, true, {}, "");
    EXPECT_EQ(clone_wo_name->NameGet(), "");

    auto clone_new_name = xnode::Clone(base_map, true, {}, "new_name");
    EXPECT_EQ(clone_new_name->NameGet(), "new_name");
}

TEST(xnode_utests, clone_empty_arr_names)
{
    auto base_arr = xnode::CreateArray({}, "test_name");
    EXPECT_EQ(base_arr->NameGet(), "test_name");

    auto clone_w_name = xnode::Clone(base_arr, true);
    EXPECT_EQ(base_arr->NameGet(), clone_w_name->NameGet());

    auto clone_wo_name = xnode::Clone(base_arr, true, {}, "");
    EXPECT_EQ(clone_wo_name->NameGet(), "");

    auto clone_new_name = xnode::Clone(base_arr, true, {}, "new_name");
    EXPECT_EQ(clone_new_name->NameGet(), "new_name");
}

TEST(xnode_utests, create_or_clone)
{
    auto arr_node = xnode::CreateArray({"123", 567}, "test_array");
    ASSERT_TRUE(arr_node);

    auto map_node = xnode::CreateMap({{"test", 12434.5}}, "test_map");
    ASSERT_TRUE(map_node);

    auto check_map = xnode::CreateOrClone(map_node, false, INode::NodeType::Map);
    ASSERT_TRUE(check_map);
    EXPECT_EQ(check_map->Type(), INode::NodeType::Map);
    EXPECT_EQ(xnode::Compare(check_map, map_node, true), 0);

    auto check_map2 = xnode::CreateOrClone(arr_node, false, INode::NodeType::Map);
    ASSERT_TRUE(check_map2);
    EXPECT_EQ(check_map2->Type(), INode::NodeType::Map);
    EXPECT_TRUE(check_map2->Empty());

    auto check_map3 = xnode::CreateOrClone(nullptr, false, INode::NodeType::Map);
    ASSERT_TRUE(check_map3);
    EXPECT_EQ(check_map3->Type(), INode::NodeType::Map);
    EXPECT_TRUE(check_map3->Empty());

    auto check_arr = xnode::CreateOrClone(arr_node, false, INode::NodeType::Array);
    ASSERT_TRUE(check_arr);
    EXPECT_EQ(check_arr->Type(), INode::NodeType::Array);
    EXPECT_EQ(xnode::Compare(check_arr, arr_node, true), 0);

    auto check_arr2 = xnode::CreateOrClone(map_node, false, INode::NodeType::Array);
    ASSERT_TRUE(check_arr2);
    EXPECT_EQ(check_arr2->Type(), INode::NodeType::Array);
    EXPECT_TRUE(check_arr2->Empty());

    auto check_arr3 = xnode::CreateOrClone(nullptr, false, INode::NodeType::Array);
    ASSERT_TRUE(check_arr3);
    EXPECT_EQ(check_arr3->Type(), INode::NodeType::Array);
    EXPECT_TRUE(check_arr3->Empty());
}

TEST(xnode_utests, empty_string) {

    auto map_node = xnode::CreateMap();
    ASSERT_TRUE(map_node);
    auto [ok,prev] = map_node->Set("empty_string", "");
    ASSERT_TRUE(ok);
    ASSERT_TRUE(!prev);
    EXPECT_EQ(map_node->Size(), 1);

    auto val = map_node->At("empty_string");
    EXPECT_EQ(val.Type(), XValue::ValueType::kString);
    EXPECT_TRUE(val.IsEmpty()) << "Empty string is not IsEmpty()";

    val = map_node->Erase("empty_string");
    EXPECT_EQ(val.Type(), XValue::ValueType::kString);
    EXPECT_TRUE(val.IsEmpty()) << "Empty string is not IsEmpty()";
    EXPECT_EQ(map_node->Size(), 0);
}

// NOLINTEND(*)