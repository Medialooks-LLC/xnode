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

TEST(xnode_tests, map_insert_and_at)
{
    IObject::SPtr  spObj  = xobject::CreateShared();
    IObject::SPtrC spObjC = std::static_pointer_cast<const IObject>(xobject::CreateShared());

    auto spMap = xnode::Create(INode::NodeType::Map);

    spMap->Insert("non_const", spObj);
    spMap->Insert("const", spObjC);

    auto xv1 = spMap->At("const");
    auto xv2 = spMap->At("non_const");

    INode::SPtrC spXNode = spMap;
    auto         xv3     = spMap->At("non_const");

#ifndef _DEBUG
    spMap->Insert("node", spXNode);
    spMap->Insert("node2", spMap);
#endif
    spMap->At("something").Bool(true);
    spMap->At("something").Bool();
}

TEST(xnode_tests, complex)
{
    const std::string kPath       = "node2::subnode1";
    const std::string kKey        = "value2";
    const char        szKey[]     = "key_char";
    auto              node_map_sp = xnode::Create(INode::NodeType::Map);

    xnode::Set(node_map_sp, XPath("node1::subnode", "next", "value"), 999);
    auto xval = xnode::At(node_map_sp, "node1::subnode::next::value");
    EXPECT_EQ(xval.Type(), XValue::kInt64);
    EXPECT_EQ(xval.Int64(), 999);

    {
        auto [success1, prev1] = xnode::Set(node_map_sp, XPath("node1::subnode", "next", "value"), 999);
        EXPECT_FALSE(success1);
        EXPECT_EQ(prev1.Int64(), 999);
    }

    {
        auto [success1, prev1] = xnode::Set(node_map_sp, XPath("node1::subnode", "next::value"), 777);
        EXPECT_TRUE(success1);
        EXPECT_EQ(prev1.Int64(), 999);
    }

    auto [success2, key2, prev2] = xnode::Insert(node_map_sp, XPath("node1", "subnode::next::value"), "999");
    EXPECT_FALSE(success2);
    EXPECT_EQ(prev2.Int64(), 777);

    xnode::Set(node_map_sp, XPath("node1::subnode2"), 777);
    xnode::Set(node_map_sp, "node1::subnode2", 999);
    xnode::Set(node_map_sp, kPath, 999);
    xnode::Set(node_map_sp, szKey, kPath);
    xnode::Set(node_map_sp, XPath(kPath, "next", "xxx", kKey), 999);
    xnode::Set(node_map_sp, XPath("node3", "next", "xxx", kKey), 999);
}

TEST(xnode_tests, clone_compare)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next", "value"), 999);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next2", "value"), 99.0);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next3::subnext", "zzz"), "vs1234");
    xnode::Set(node_map_sp, "node1::xxx::value", 99.0);
    xnode::Set(node_map_sp, "int_val", 10);
    xnode::Set(node_map_sp, "uint_val", 10U); // todo: ULL for gcc
    // todo: ULL for gcc
    // error: conversion from 'long long unsigned int' to 'INode::XValue' is ambiguous
    // xnode::Set(node_map_sp, "uint64_val", 10ULL);
    xnode::Set(node_map_sp, "bool_val", true);
    xnode::Set(node_map_sp, "str_val", "99.0");
    xnode::Set(node_map_sp, "dbl_val", M_PI);

    IObject::SPtr  spObj  = xobject::CreateShared();
    IObject::SPtrC spObjC = std::static_pointer_cast<const IObject>(xobject::CreateShared());
    xnode::Set(node_map_sp, "objects::1", spObj);
    xnode::Set(node_map_sp, "objects::2", spObjC);

    auto node_clone = xnode::Clone(node_map_sp, true);
    ASSERT_TRUE(node_clone && node_clone->Size() == node_map_sp->Size());

    auto cmp_res = xnode::Compare(
        node_map_sp,
        node_clone,
        true,
        [](const INode::SPtrC& ncp, const XKey& key, const XValueRT& left, const XValueRT& right) { return true; });
    EXPECT_EQ(cmp_res, 0);

    node_map_sp->Erase("dbl_val");
    cmp_res = xnode::Compare(node_map_sp, node_clone, true);
    EXPECT_LT(cmp_res, 0);

    cmp_res = xnode::Compare(node_clone, node_map_sp, true);
    EXPECT_GT(cmp_res, 0);

    node_clone->Set("dbl_val", XValue());
    cmp_res = xnode::Compare(node_clone, node_map_sp, true);
    EXPECT_EQ(cmp_res, 0);
}


TEST(xnode_tests, clone_with_const_node)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next", "value"), 999);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next2", "value"), 99.0);
    xnode::Set(node_map_sp, XPath("node1::subnode", "next3::subnext", "zzz"), "vs1234");
    xnode::Set(node_map_sp, "node1::xxx::value", 99.0);
    xnode::Set(node_map_sp, "int_val", 10);
    xnode::Set(node_map_sp, "uint_val", 10U); // todo: ULL for gcc

    auto node_child = xnode::CreateComplex({{"a", "134"}, {"b", "345"}, {"subnode::c", 99}}, "const_node");
    EXPECT_EQ(xnode::At(node_child, {"subnode", "c"}).Int32(), 99);

    xnode::NodeConstInsert(node_map_sp, node_child, true);

    // todo: ULL for gcc
    // error: conversion from 'long long unsigned int' to 'INode::XValue' is ambiguous
    // xnode::Set(node_map_sp, "uint64_val", 10ULL);
    xnode::Set(node_map_sp, "bool_val", true);
    xnode::Set(node_map_sp, "str_val", "99.0");
    xnode::Set(node_map_sp, "dbl_val", M_PI);

    auto node_clone = xnode::Clone(std::const_pointer_cast<const INode>(node_map_sp), true);
    ASSERT_TRUE(node_clone && node_clone->Size() == node_map_sp->Size());

    auto cmp_res = xnode::Compare(
        node_map_sp,
        node_clone,
        true,
        [](const INode::SPtrC& ncp, const XKey& key, const XValueRT& left, const XValueRT& right) { return true; });
    EXPECT_EQ(cmp_res, 0);

    node_map_sp->Erase("dbl_val");
    cmp_res = xnode::Compare(node_map_sp, node_clone, true);
    EXPECT_LT(cmp_res, 0);

    cmp_res = xnode::Compare(node_clone, node_map_sp, true);
    EXPECT_GT(cmp_res, 0);

    node_clone->Set("dbl_val", XValue());
    cmp_res = xnode::Compare(node_clone, node_map_sp, true);
    EXPECT_EQ(cmp_res, 0);
}

TEST(xnode_tests, path_tests)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    auto [ok, xval]  = xnode::Set(node_map_sp, XPath("node1::subnode", "next::value"), 999);
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kEmpty);

    static const std::string      kBase    = "node1::subnode";
    static const std::string_view kNext    = "next";
    static const std::string      kValue   = "value";
    static const std::string      kCombStr = "next::value";
    static const std::string_view kCombSV  = "next::value";
    static const char*            kCombPSZ = "next::value";

    xval = xnode::At(node_map_sp, XPath(kBase, kNext, kValue));
    EXPECT_EQ(xval.Type(), XValue::kInt64);
    EXPECT_EQ(xval.Int64(), 999);

    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBase, kCombStr)), 999);
    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBase, kCombSV)), 999);
    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBase, kCombPSZ)), 999);

    static const std::string kBaseSV  = "node1::subnode";
    static const char*       kBasePSZ = "node1::subnode";
    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBaseSV, kCombStr)), 999);
    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBasePSZ, kCombSV)), 999);
    EXPECT_EQ(xnode::At(node_map_sp, XPath(kBasePSZ, kCombPSZ)), 999);

    std::tie(ok, xval) = xnode::Set(node_map_sp, XPath("node1::subnode", "next2", "value"), 99.0);
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kEmpty);

    xval = xnode::At(node_map_sp, XPath("node1", "subnode::next2::value"));
    EXPECT_EQ(xval.Type(), XValue::kDouble);
    EXPECT_EQ(xval.Double(), 99.0);

    std::tie(ok, xval) = xnode::Set(node_map_sp, XPath("node1::subnode", "next3::subnext::zzz"), "vs1234");
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kEmpty);
    std::tie(ok, xval) = xnode::Set(node_map_sp, XPath("node1", "subnode::next3::subnext", "zzz"), "vs1235");
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kString);
    EXPECT_EQ(xval.String(), "vs1234");
    xval = xnode::At(node_map_sp, "node1::subnode::next3::subnext::zzz");
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kString);
    EXPECT_EQ(xval.String(), "vs1235");

    std::tie(ok, xval) = xnode::Set(node_map_sp, "node1::xxx::value", 77.3);
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kEmpty);
    std::tie(ok, xval) = xnode::Set(node_map_sp, XPath("node1::xxx::value"), 79.5);
    EXPECT_TRUE(ok);
    EXPECT_EQ(xval.Type(), XValue::kDouble);
    EXPECT_EQ(xval.Double(), 77.3);

    xval = xnode::At(node_map_sp, "node1::xxx::value");
    EXPECT_EQ(xval.Type(), XValue::kDouble);
    EXPECT_EQ(xval.Double(), 79.5);

    xnode::Set(node_map_sp, "int_val", 10);
    EXPECT_EQ(xnode::At(node_map_sp, "int_val"), 10);
    EXPECT_EQ(xnode::At(node_map_sp, "int_val").Type(), XValue::kInt64);

    xnode::Set(node_map_sp, "uint_val", 10U); // todo: ULL for gcc
    EXPECT_EQ(xnode::At(node_map_sp, "uint_val"), 10);
    EXPECT_EQ(xnode::At(node_map_sp, "uint_val").Type(), XValue::kUint64);
}

TEST(xnode_tests, path_tests_parse_string)
{
    std::vector<std::string_view> check_paths = {"node::subnode[12]::value",
                                                 "node::subnode[12]value",
                                                 "node[]subnode::[12]value",
                                                 "node::subnode::[12]::value",
                                                 "[node]subnode[12]value::",
                                                 "node::[subnode][12]value",
                                                 "node[subnode][12]value",
                                                 "::[node][subnode][12]::::[value",
                                                 "node::[subnode][12][value]",
                                                 "::node::[subnode]::[12][value]"};

    for (const auto& path : check_paths) {

        auto xpath = XPath(path);
        EXPECT_EQ(xpath.size(), 4);
        EXPECT_EQ(xpath.at(0).StringGet().value_or(""), "node");
        EXPECT_EQ(xpath.at(1).StringGet().value_or(""), "subnode");
        EXPECT_EQ(xpath.at(2).IndexGet().value_or(0), 12);
        EXPECT_EQ(xpath.at(3).StringGet().value_or(""), "value");

        auto str_path = xpath.to_string();
        EXPECT_EQ(str_path, "node::subnode[12]::value");
    }
}

TEST(xnode_tests, path_tests_parse_string2)
{
    std::vector<std::string_view> check_paths = {"node[allow::dots]value::",
                                                 "::node::[allow::dots]::value",
                                                 "::node::[allow::dots]::value[]",
                                                 "node[allow::dots]::::[value]",
                                                 "node::[allow::dots][value"};

    for (const auto& path : check_paths) {

        auto xpath = XPath(path);
        EXPECT_EQ(xpath.size(), 3);
        EXPECT_EQ(xpath.at(0).StringGet().value_or(""), "node");
        EXPECT_EQ(xpath.at(1).StringGet().value_or(""), "allow::dots");
        EXPECT_EQ(xpath.at(2).StringGet().value_or(""), "value");

        auto str_path = xpath.to_string();
        EXPECT_EQ(str_path, "node[allow::dots]::value");
    }
}

TEST(xnode_tests, path_node_tests)
{
    auto node_map_sp     = xnode::Create(INode::NodeType::Map);
    auto child_map_sp    = xnode::NodeGet(node_map_sp, XPath("node1::subnode", "next", "node"), INode::NodeType::Map);
    auto parent_map_sp_c = xnode::NodeConstGet(INode::SPtrC(node_map_sp), XPath("node1::subnode", "next"));
    ASSERT_TRUE(child_map_sp.get());
    ASSERT_TRUE(parent_map_sp_c.get());
    EXPECT_EQ(parent_map_sp_c->At("node").ObjectPtrC(), child_map_sp);
    EXPECT_FALSE(parent_map_sp_c->At("node").ObjectPtr());

    child_map_sp->Set("value", 1234.5);
    auto xval = xnode::At(parent_map_sp_c, "node::value");
    EXPECT_EQ(xval.Type(), XValue::kDouble);
    EXPECT_EQ(xval.Double(), 1234.5);

    xval = xnode::At(node_map_sp, "node1::subnode::next::node::value");
    EXPECT_EQ(xval.Type(), XValue::kDouble);
    EXPECT_EQ(xval.Double(), 1234.5);

    EXPECT_TRUE(xnode::At(node_map_sp, "node1::subnode::next::node::value2").IsEmpty());
    EXPECT_TRUE(xnode::At(node_map_sp, "node2::subnode::next::node::value").IsEmpty());
}

TEST(xnode_tests, node_name_parent_tests)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    auto child_sp    = xnode::NodeGet(node_map_sp, XPath("node1::subnode", "next", "node"), INode::NodeType::Map);
    auto parent_sp_c = xnode::NodeConstGet(INode::SPtrC(node_map_sp), XPath("node1::subnode", "next"));
    ASSERT_TRUE(child_sp.get());
    ASSERT_TRUE(parent_sp_c.get());
    EXPECT_EQ(parent_sp_c->At("node").ObjectPtrC(), child_sp);
    EXPECT_EQ(parent_sp_c->At("node").ObjectPtr(), nullptr);

    EXPECT_TRUE(child_sp->IsName("node"));
    EXPECT_EQ(child_sp->NameGet(), "node");

    // Check name changing & parent update

    // Should not set (have parent)
    auto [is_set, prev_name] = child_sp->NameSet("node2", false);
    EXPECT_FALSE(is_set);
    EXPECT_EQ(prev_name, "node");
    EXPECT_EQ(child_sp->NameGet(), "node");

    // Should set (update parent)
    std::tie(is_set, prev_name) = child_sp->NameSet("node2", true);
    EXPECT_TRUE(is_set);
    EXPECT_EQ(prev_name, "node");
    EXPECT_TRUE(child_sp->IsName("node2"));
    EXPECT_EQ(child_sp->NameGet(), "node2");

    // Check changes in parent
    EXPECT_TRUE(parent_sp_c->At("node").IsEmpty());
    EXPECT_EQ(parent_sp_c->At("node2").ObjectPtrC(), child_sp);

    auto parent2_sp = xnode::Create(INode::NodeType::Map);
    parent2_sp->Set("node2", "1234");

    // Move to other node

    // Shlould be failed (already have "node2" attribute)
    auto InsertRes = xnode::NodeInsert(parent2_sp, child_sp, false);
    EXPECT_FALSE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), "node2");
    EXPECT_EQ(InsertRes.existed, "1234");

    // Should be ok
    InsertRes = xnode::NodeInsert(parent2_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), child_sp->NameGet());
    EXPECT_TRUE(InsertRes.existed.IsEmpty());

    // Check moving from previous parent
    EXPECT_TRUE(parent_sp_c->At("node2").IsEmpty());
    EXPECT_EQ(parent_sp_c->Size(), 0);

    // Check new parent
    EXPECT_EQ(parent2_sp->At("node2"), child_sp);
    EXPECT_EQ(parent2_sp, child_sp->ParentGet());

    // Detach from parent
    auto parent_sp = child_sp->ParentDetach();
    EXPECT_EQ(parent2_sp, parent_sp);
    EXPECT_EQ(parent2_sp->Size(), 0);
    EXPECT_EQ(child_sp->ParentGet(), nullptr);

    // Update name (no parent now), should be ok
    std::tie(is_set, prev_name) = child_sp->NameSet("node3", false);
    EXPECT_TRUE(is_set);
    EXPECT_EQ(prev_name, "node2");
    EXPECT_TRUE(child_sp->IsName("node3"));

    // Should be ok
    InsertRes = xnode::NodeInsert(parent2_sp, child_sp, false);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), child_sp->NameGet());
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    EXPECT_EQ(parent2_sp->At("node3"), child_sp);
    EXPECT_EQ(parent2_sp, child_sp->ParentGet());

    // Try failed rename (same name already exists)
    parent2_sp->Set("node4", 567);
    std::tie(is_set, prev_name) = child_sp->NameSet("node4", true);
    EXPECT_FALSE(is_set);
    EXPECT_EQ(prev_name, "node3");
    EXPECT_TRUE(child_sp->IsName("node3"));

    // Check what note still with parent
    EXPECT_EQ(parent2_sp->At("node3"), child_sp);
    EXPECT_EQ(parent2_sp, child_sp->ParentGet());

    // Check for changin name via key
    auto ok = parent2_sp->KeyChange("node3", "new_name");
    EXPECT_TRUE(ok);
    EXPECT_TRUE(child_sp->IsName("new_name"));
    EXPECT_EQ(parent2_sp->At("new_name"), child_sp);
    EXPECT_EQ(parent2_sp, child_sp->ParentGet());

    EXPECT_TRUE(parent2_sp->At("node3").IsEmpty());

    // Change expect to be failed
    ok = parent2_sp->KeyChange("new_name", "node4");
    EXPECT_FALSE(ok);
    EXPECT_TRUE(child_sp->IsName("new_name"));
    EXPECT_EQ(parent2_sp->At("new_name"), child_sp);
    EXPECT_EQ(parent2_sp, child_sp->ParentGet());

    EXPECT_EQ(parent2_sp->At("node4"), 567);
}

TEST(xnode_tests, node_array_parent_tests)
{
    auto node_map_sp  = xnode::Create(INode::NodeType::Map);
    auto node_arr1_sp = xnode::Create(INode::NodeType::Array);
    auto node_arr2_sp = xnode::Create(INode::NodeType::Array);
    auto child_sp     = xnode::Create(INode::NodeType::Map, "child_node");
    child_sp->Set("val", 1234);

    // Should be ok
    auto InsertRes = xnode::NodeInsert(node_map_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), child_sp->NameGet());
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    EXPECT_EQ(node_map_sp->At("child_node"), child_sp);
    EXPECT_EQ(node_map_sp, child_sp->ParentGet());
    EXPECT_EQ(node_map_sp->Size(), 1);

    // Insert const node
    InsertRes = xnode::NodeConstInsert(node_map_sp, child_sp, true, "const_child");
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), "const_child");
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 2);

    // Insert to array
    InsertRes = xnode::NodeInsert(node_arr1_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(10), 0);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    EXPECT_EQ(node_arr1_sp->At(0), child_sp);
    EXPECT_EQ(node_arr1_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr1_sp->Size(), 1);

    // Check moving from previous parent
    EXPECT_TRUE(node_map_sp->At("child_node").IsEmpty());
    // But keep const !!!
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 1);

    // Insert to next array
    InsertRes = xnode::NodeInsert(node_arr2_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(10), 0);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 1);

    // Check moving from previous array
    EXPECT_TRUE(node_arr1_sp->At(0).IsEmpty());
    EXPECT_EQ(node_arr1_sp->Size(), 0);

    // For have non empty array
    node_arr2_sp->Insert(kIdxEnd, 1234);
    EXPECT_EQ(node_arr2_sp->At(1), 1234);

    // Insert as constant in same array (should be added)
    InsertRes = xnode::NodeConstInsert(node_arr2_sp, child_sp, true);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(0), 2);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    // Keep node
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp->At(1), 1234);
    // Check const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(2)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(2), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 3);

    // Insert again in same array - should be failed
    InsertRes = xnode::NodeInsert(node_arr2_sp, child_sp, true);
    EXPECT_FALSE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(-1), 0);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    // Check what prev entry (1) - not removed
    EXPECT_EQ(node_arr2_sp->At(1), 1234);
    // But keep const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(2)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(2), child_sp);
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 3);

    // Insert to map
    InsertRes = xnode::NodeInsert(node_map_sp, child_sp, true);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), child_sp->NameGet());
    EXPECT_TRUE(InsertRes.existed.IsEmpty());

    EXPECT_EQ(node_map_sp->At("child_node"), child_sp);
    EXPECT_EQ(node_map_sp, child_sp->ParentGet());
    // Const should be unchanged
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 2);

    // Check moving from previous array
    EXPECT_EQ(node_arr2_sp->At(0), 1234);
    // But keep const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(1)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(1), child_sp);
    EXPECT_TRUE(node_arr2_sp->At(2).IsEmpty());
    EXPECT_EQ(node_arr2_sp->Size(), 2);
}

TEST(xnode_tests, node_array_parent_set)
{
    auto node_map_sp  = xnode::Create(INode::NodeType::Map);
    auto node_arr1_sp = xnode::Create(INode::NodeType::Array);
    auto node_arr2_sp = xnode::Create(INode::NodeType::Array);
    auto child_sp     = xnode::Create(INode::NodeType::Map, "child_node");
    child_sp->Set("val", 1234);

    // Should be ok
    auto [ok, prev_p] = child_sp->ParentSet(node_map_sp);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(prev_p);
    EXPECT_EQ(node_map_sp->At("child_node"), child_sp);
    EXPECT_EQ(node_map_sp, child_sp->ParentGet());
    EXPECT_EQ(node_map_sp->Size(), 1);

    // Insert const node
    auto [success, key, val] = xnode::NodeConstInsert(node_map_sp, child_sp, true, "const_child");
    EXPECT_TRUE(success);
    EXPECT_EQ(key.StringGet().value_or(""), "const_child");
    EXPECT_TRUE(val.IsEmpty());
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 2);

    // Insert to array
    std::tie(ok, prev_p) = child_sp->ParentSet(node_arr1_sp);
    EXPECT_TRUE(ok);
    EXPECT_EQ(prev_p, node_map_sp);
    EXPECT_EQ(node_arr1_sp->At(0), child_sp);
    EXPECT_EQ(node_arr1_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr1_sp->Size(), 1);

    // Check moving from previous parent
    EXPECT_TRUE(node_map_sp->At("child_node").IsEmpty());
    // But keep const !!!
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 1);

    // Insert to next array
    std::tie(ok, prev_p) = child_sp->ParentSet(node_arr2_sp);
    EXPECT_TRUE(ok);
    EXPECT_EQ(prev_p, node_arr1_sp);
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 1);

    // Check moving from previous array
    EXPECT_TRUE(node_arr1_sp->At(0).IsEmpty());
    EXPECT_EQ(node_arr1_sp->Size(), 0);

    // For have non empty array
    node_arr2_sp->Insert(kIdxEnd, 1234);
    EXPECT_EQ(node_arr2_sp->At(1), 1234);

    // Insert as constant in same array (should be added)
    auto InsertRes = xnode::NodeConstInsert(node_arr2_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(0), 2);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    // Keep node
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp->At(1), 1234);
    // Check const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(2)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(2), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 3);

    // Insert again in same array -> should be failed
    InsertRes = xnode::NodeInsert(node_arr2_sp, child_sp, true);
    EXPECT_FALSE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.IndexGet().value_or(-1), 0);
    EXPECT_TRUE(InsertRes.existed.IsEmpty());
    // Check what prev entry (1) ok
    EXPECT_EQ(node_arr2_sp->At(1), 1234);
    // But keep const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(2)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(2), child_sp);
    EXPECT_EQ(node_arr2_sp->At(0), child_sp);
    EXPECT_EQ(node_arr2_sp, child_sp->ParentGet());
    EXPECT_EQ(node_arr2_sp->Size(), 3);

    // Insert to map
    InsertRes = xnode::NodeInsert(node_map_sp, child_sp, true);
    EXPECT_TRUE(InsertRes.succeeded);
    EXPECT_EQ(InsertRes.inserted_at.StringGet().value_or(""), child_sp->NameGet());
    EXPECT_TRUE(InsertRes.existed.IsEmpty());

    EXPECT_EQ(node_map_sp->At("child_node"), child_sp);
    EXPECT_EQ(node_map_sp, child_sp->ParentGet());
    // Const should be unchanged
    EXPECT_EQ(node_map_sp->At("const_child"), child_sp);
    EXPECT_EQ(xnode::NodeTypeGet(node_map_sp->At("const_child")), xnode::XNodeType::const_map);
    EXPECT_EQ(node_map_sp->Size(), 2);

    // Check moving from previous array
    EXPECT_EQ(node_arr2_sp->At(0), 1234);
    // But keep const !
    EXPECT_EQ(xnode::NodeTypeGet(node_arr2_sp->At(1)), xnode::XNodeType::const_map);
    EXPECT_EQ(node_arr2_sp->At(1), child_sp);
    EXPECT_TRUE(node_arr2_sp->At(2).IsEmpty());
    EXPECT_EQ(node_arr2_sp->Size(), 2);
}

TEST(xnode_tests, node_parents_test_w_clear)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    auto node_arr_sp = xnode::Create(INode::NodeType::Array);

    node_map_sp->Set("1234", 99.0);
    node_arr_sp->Insert(0, "value_start");
    node_arr_sp->Insert(kIdxEnd, "value_end");
    std::vector<INode::SPtrC> vec_childs;

    size_t test_childs = 23;
    for (size_t z = 0; z < test_childs; ++z) {
        auto child_sp = xnode::Create(INode::NodeType::Map, "child_node_" + std::to_string(z));
        child_sp->Set("idx", z);

        switch (z % 8) {
            case 0: {
                bool ok = child_sp->ParentSet(node_map_sp).first;
                EXPECT_TRUE(ok);
                break;
            }
            case 1: {
                bool ok = child_sp->ParentSet(node_arr_sp).first;
                EXPECT_TRUE(ok);
                break;
            }
            case 2: {
                bool ok = node_map_sp->Set(std::to_string(z), child_sp).first;
                EXPECT_TRUE(ok);
                break;
            }
            case 3: {
                bool ok = node_arr_sp->Set(z, child_sp).first;
                EXPECT_TRUE(ok);
                break;
            }
            case 4: {
                auto [ok, key, val] = node_map_sp->Insert(std::to_string(z), child_sp);
                EXPECT_TRUE(ok);
                break;
            }
            case 5: {
                auto [ok, key, val] = node_arr_sp->Insert(z, child_sp);
                EXPECT_TRUE(ok);
                break;
            }
            case 6: {
                auto [ok, key, val] = xnode::NodeInsert(node_map_sp, child_sp, true);
                EXPECT_TRUE(ok);
                break;
            }
            case 7: {
                auto [ok, key, val] = xnode::NodeInsert(node_arr_sp, child_sp, true);
                EXPECT_TRUE(ok);
                break;
            }
        }

        EXPECT_EQ(child_sp->ParentGet(), z % 2 ? node_arr_sp : node_map_sp);

        vec_childs.push_back(child_sp);
    }

    // auto str          = xnode::ToJson(node_map_sp);
    auto node_map_sp2 = xnode::Clone(node_map_sp, true);
    auto str          = xnode::ToJson(node_map_sp);
    auto str2         = xnode::ToJson(node_map_sp2);
    EXPECT_EQ(str, str2);

    auto str3         = xnode::ToJson(node_arr_sp);
    auto node_arr_sp2 = xnode::Clone(node_arr_sp, true);
    auto str4         = xnode::ToJson(node_arr_sp2);
    EXPECT_EQ(str3, str4);

    auto cmp = xnode::Compare(node_map_sp, node_map_sp2, true);
    EXPECT_EQ(cmp, 0);
    cmp = xnode::Compare(node_arr_sp, node_arr_sp2, true);
    // EXPECT_EQ(cmp, 0); // TODO !!!!

    node_map_sp->Clear();
    node_arr_sp->Clear();

    for (auto child_sp : vec_childs)
        EXPECT_FALSE(child_sp->ParentGet());

    vec_childs.clear();

    node_map_sp2->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        auto node_p = val.QueryPtrC<INode>();
        if (node_p)
            vec_childs.push_back(node_p);
        return OnCopyRes::Skip;
    });

    node_arr_sp2->BulkGetAll([&](const XKey& key, const XValueRT& val) {
        auto node_p = val.QueryPtrC<INode>();
        if (node_p)
            vec_childs.push_back(node_p);

        return OnCopyRes::Skip;
    });

    EXPECT_EQ(vec_childs.size(), test_childs);

    for (auto child_sp : vec_childs) {
        EXPECT_EQ(child_sp->At("idx").Type(), XValue::kUint64);
        auto idx = child_sp->At("idx").Uint64();
        EXPECT_EQ(child_sp->ParentGet(), idx % 2 ? node_arr_sp2 : node_map_sp2);
    }

    size_t idx = 0;
    node_map_sp2->ForEach([&](const auto& key, auto& val) {
        if (idx % 2)
            val = XValue(1234);

        return idx++ % 2 ? OnEachRes::Next : OnEachRes::Erase;
    });

    node_arr_sp2->ForEach([&](const auto& key, auto& val) {
        if (idx % 2)
            val = XValue(1234);

        return idx++ % 2 ? OnEachRes::Next : OnEachRes::Erase;
    });

    for (auto child_sp : vec_childs)
        EXPECT_FALSE(child_sp->ParentGet());
}

TEST(xnode_tests, json_tests)
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
    size_t zDiff   = 0;
    auto   cmp_res = xnode::Compare(
        node_map_sp,
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

TEST(xnode_tests, array_tests)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    auto success     = xnode::Set(node_map_sp, XPath("node::array", 5), 999).first;
    EXPECT_TRUE(success);
    success = xnode::Set(node_map_sp, XPath("node::array", 5000), 999).first;
    EXPECT_FALSE(success);
    auto chk = xnode::At(node_map_sp, XPath("node::array", 5));
    EXPECT_EQ(chk.Int64(), 999);
    auto [success2, key, fake] = xnode::Insert(node_map_sp, XPath("node::array", 5), 777);
    EXPECT_TRUE(success2);
    EXPECT_EQ(key.IndexGet().value_or(0), 5);
    EXPECT_EQ(xnode::At(node_map_sp, XPath("node::array", 5)).Int64(), 777);

    EXPECT_EQ(xnode::At(node_map_sp, XPath("node::array", kIdxLast)).Int64(), 999);
    xnode::Set(node_map_sp, XPath("node::array", kIdxLast), 888);
    EXPECT_EQ(xnode::At(node_map_sp, XPath("node::array", kIdxLast)).Int64(), 888);

    auto str = xnode::ToJson(node_map_sp, nullptr, xnode::JsonFormat::kOneLineArrays);
    std::cout << "ORIGINAL:" << str << std::endl;
}

TEST(xnode_tests, map_index_access)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    node_map_sp->Set("0_first", "first");
    node_map_sp->Set("1_second", "second");
    node_map_sp->Set("2_third", "third");

    EXPECT_TRUE(node_map_sp->At("0_first") == node_map_sp->At(0));
    EXPECT_TRUE(node_map_sp->At("1_second") == node_map_sp->At(1));
    EXPECT_TRUE(node_map_sp->At("2_third") == node_map_sp->At(2));

    EXPECT_TRUE(node_map_sp->At("0_first") == node_map_sp->At(kIdxBegin));

    auto xval = node_map_sp->At(kIdxLast);
    EXPECT_TRUE(node_map_sp->At("2_third") == node_map_sp->At(kIdxLast));

    node_map_sp->Set(1, "second_udp");
    EXPECT_EQ(node_map_sp->At("1_second").String(), "second_udp");
}

TEST(xnode_tests, elapsed_time)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);

    auto t = node_map_sp->Set("0", "first").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);

    auto [fail, current] = node_map_sp->Set("0", "first");
    EXPECT_EQ(fail, false);
    EXPECT_EQ(current.String(), "first");
    EXPECT_GT(current.TimeElapsed(), 0);
    EXPECT_LT(current.TimeElapsed(), XValueRT::MsecToTicks(1));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto [ok, prev] = node_map_sp->Set("0", "_first");
    EXPECT_EQ(ok, true);
    EXPECT_EQ(prev.String(), "first");
    EXPECT_GT(prev.TimeElapsed(), XValueRT::MsecToTicks(10));
    EXPECT_LT(prev.TimeElapsed(), XValueRT::MsecToTicks(100));

    EXPECT_GT(node_map_sp->At("0").TimeElapsed(), 0);
    EXPECT_LT(node_map_sp->At("0").TimeElapsed(), XValueRT::MsecToTicks(1));
}

TEST(xnode_tests, erasing_elapsed)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);

    auto t = node_map_sp->Set("0", "first").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("1", "second").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("2", "third").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);

    EXPECT_EQ(node_map_sp->Size(), 3);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto erased = node_map_sp->Erase("0");
    EXPECT_FALSE(erased.TimeIsAbsent());
    EXPECT_GT(erased.TimeElapsed(), XValueRT::MsecToTicks(10));

    EXPECT_EQ(node_map_sp->Size(), 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto [ok, prev] = node_map_sp->Set("0", "_first");
    EXPECT_EQ(ok, true);
    EXPECT_EQ(prev.Type(), XValue::kEmpty);
    EXPECT_GT(prev.TimeElapsed(), XValueRT::MsecToTicks(10));
    EXPECT_LT(prev.TimeElapsed(), XValueRT::MsecToTicks(100));

    EXPECT_EQ(node_map_sp->Size(), 3);

    auto [del, removed] = node_map_sp->Set("1", XValue());
    EXPECT_EQ(del, true);
    EXPECT_EQ(removed.String(), "second");
    EXPECT_EQ(node_map_sp->Size(), 2);

    EXPECT_GT(removed.TimeElapsed(), XValueRT::MsecToTicks(20));
    EXPECT_LT(removed.TimeElapsed(), XValueRT::MsecToTicks(100));

    EXPECT_GT(node_map_sp->At("1").TimeElapsed(), 0);
    EXPECT_LT(node_map_sp->At("1").TimeElapsed(), XValueRT::MsecToTicks(1));
}

TEST(xnode_tests, erasing_history)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);

    auto t = node_map_sp->Set("0", "first").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("1", "second").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("2", "third").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);

    EXPECT_EQ(node_map_sp->Size(), 3);

    node_map_sp->Erase("0"); // 300 msec
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    node_map_sp->Erase("1"); // 200 msec
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    node_map_sp->Erase("2"); // 100 msec
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(node_map_sp->Empty());
    EXPECT_EQ(node_map_sp->Size(), 0);

    auto values = node_map_sp->BulkGetAll();
    EXPECT_EQ(values.size(), 0);

    size_t counter = 0;
    bool   found   = node_map_sp->ForEach([&](const XKey&, XValueRT&) {
        ++counter;
        return OnEachRes::Next;
    });
    EXPECT_FALSE(found);
    ASSERT_EQ(counter, 0);

    int64_t arrMin[] = {XValueRT::MsecToTicks(300), XValueRT::MsecToTicks(200), XValueRT::MsecToTicks(100)};
    found            = node_map_sp->ForPatch([&](const XKey&, const XValueRT& val) {
        EXPECT_GT(val.TimeElapsed(), arrMin[counter]);
        EXPECT_LT(val.TimeElapsed(), arrMin[counter] + XValueRT::MsecToTicks(50));
        ++counter;
        return false;
    });
    EXPECT_TRUE(found);
    EXPECT_EQ(counter, 3);
}

TEST(xnode_tests, erasing_via_each_history)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);

    auto t = node_map_sp->Set("0", "first").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("1", "second").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);
    t = node_map_sp->Set("2", "third").second.Timestamp();
    EXPECT_EQ(t, kAbsentRT);

    EXPECT_EQ(node_map_sp->Size(), 3);

    node_map_sp->ForEach([](auto&&...) { return OnEachRes::Erase; });

    EXPECT_TRUE(node_map_sp->Empty());
    EXPECT_EQ(node_map_sp->Size(), 0);

    auto values = node_map_sp->BulkGetAll();
    EXPECT_EQ(values.size(), 0);

    size_t counter = 0;
    bool   found   = node_map_sp->ForEach([&](const XKey&, XValueRT&) {
        ++counter;
        return OnEachRes::Next;
    });
    EXPECT_FALSE(found);
    EXPECT_EQ(counter, 0);

    found = node_map_sp->ForPatch([&](const XKey&, const XValueRT&) {
        ++counter;
        return false;
    });
    EXPECT_TRUE(found);
    EXPECT_EQ(counter, 3);
}

TEST(xnode_tests, node_parent_same) {
#ifndef _DEBUG
    {auto node_map_sp = xnode::Create(INode::NodeType::Map, "root");
auto [ok, prev] = node_map_sp->ParentSet(node_map_sp);
EXPECT_EQ(ok, false);
EXPECT_EQ(prev, node_map_sp);

ok = node_map_sp->Set("123", node_map_sp).first;
EXPECT_EQ(ok, false);

auto res = node_map_sp->Insert("1234", node_map_sp);
EXPECT_EQ(res.succeeded, false);
}
#endif
// ASSERT_EQ(impl::XNode::counter(), 0);
}
TEST(xnode_tests, node_parent_circular_set)
{
#ifndef _DEBUG
    {
        auto node_map_sp = xnode::Create(INode::NodeType::Map, "root");

        node_map_sp->ParentSet(node_map_sp);

        auto node_map_sp1 = xnode::CreateMap({{"name1", "01"}}, "01");
        auto node_map_sp2 = xnode::CreateMap({{"name2", "02"}}, "02");
        auto node_map_sp3 = xnode::CreateMap({{"name3", "03"}}, "03");

        auto [ok, prev] = node_map_sp1->ParentSet(node_map_sp);
        EXPECT_TRUE(ok);
        std::tie(ok, prev) = node_map_sp2->ParentSet(node_map_sp1);
        EXPECT_TRUE(ok);
        std::tie(ok, prev) = node_map_sp3->ParentSet(node_map_sp2);
        EXPECT_TRUE(ok);

        std::tie(ok, prev) = node_map_sp->ParentSet(node_map_sp3);
        EXPECT_FALSE(ok);
        EXPECT_EQ(prev, node_map_sp3);
        std::tie(ok, prev) = node_map_sp1->ParentSet(node_map_sp3);
        EXPECT_EQ(ok, false);
        EXPECT_EQ(prev, node_map_sp3);

        auto [is_set, val] = node_map_sp3->Set("root_2", node_map_sp);
        EXPECT_FALSE(is_set);
        EXPECT_FALSE(node_map_sp->ParentGet());

        auto res = node_map_sp2->Insert("root_2", node_map_sp);
        EXPECT_FALSE(res.succeeded);
        EXPECT_FALSE(node_map_sp->ParentGet());

        // Check for const node
        res = xnode::NodeConstInsert(node_map_sp2, node_map_sp, true);
        EXPECT_FALSE(res.succeeded);
        EXPECT_FALSE(node_map_sp->ParentGet());

        // check for bulk
        auto inserted = node_map_sp2->BulkInsert({{"test_node", node_map_sp}});
        EXPECT_EQ(inserted, 0);
        EXPECT_FALSE(node_map_sp->ParentGet());

        auto set_count = node_map_sp2->BulkSet({{"test_node", node_map_sp}});
        EXPECT_EQ(set_count, 0);
        EXPECT_FALSE(node_map_sp->ParentGet());

        // check cb
        node_map_sp2->Set("place_for_node", 123);
        node_map_sp2->ForEach([&](const auto& key, auto& val) {
            if (key == XKey("place_for_node")) {
                val = XValue(node_map_sp);
                return OnEachRes::Stop;
            }
            return OnEachRes::Next;
        });

        // EXPECT_EQ(node_map_sp2->At("place_for_node"), 123);
        EXPECT_FALSE(node_map_sp->ParentGet());
    }
#endif

    // ASSERT_EQ(impl::XNode::counter(), 0);
}

// NOLINTEND(*)
