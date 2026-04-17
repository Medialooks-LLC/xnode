#include "xnode.h"

#include <gtest/gtest.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>
#include <string>
#include <thread>

namespace xsdk::xnode::json_test {

// NOLINTBEGIN(*)

constexpr uint64_t kJsonSafeUIntMax = (uint64_t {1} << 53) - 1;
constexpr int64_t  kJsonSafeIntMax  = (int64_t {1} << 53) - 1;
constexpr int64_t  kJsonSafeIntMin  = -kJsonSafeIntMax;

constexpr uint64_t kJsonUnsafeUInt   = (uint64_t {1} << 53);
constexpr int64_t  kJsonUnsafePosInt = (int64_t {1} << 53);
constexpr int64_t  kJsonUnsafeNegInt = -((int64_t {1} << 53));

std::string Quote(const std::string& _value) { return "\"" + _value + "\""; }

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
    node_array->Insert(xnode::kIdxEnd, 123);
    node_array->Insert(xnode::kIdxEnd, "string");
    node_array->Insert(xnode::kIdxEnd, 123.456);
    node_array->Insert(xnode::kIdxEnd, nullptr);
    node_array->Insert(xnode::kIdxEnd, true);
    node_array->Insert(xnode::kIdxEnd, kJsonSafeUIntMax);
    node_array->Insert(xnode::kIdxEnd, kJsonSafeIntMin);
    node_array->Insert(xnode::kIdxBegin, "first");
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

TEST(xnode_tests_json, safe_53bit_root_scalars_are_serialized_as_numbers)
{
    EXPECT_EQ(xnode::ToJson(XValue(kJsonSafeIntMax), nullptr, xnode::JsonFormat::kOneLine),
              std::to_string(kJsonSafeIntMax));
    EXPECT_EQ(xnode::ToJson(XValue(kJsonSafeIntMin), nullptr, xnode::JsonFormat::kOneLine),
              std::to_string(kJsonSafeIntMin));
    EXPECT_EQ(xnode::ToJson(XValue(kJsonSafeUIntMax), nullptr, xnode::JsonFormat::kOneLine),
              std::to_string(kJsonSafeUIntMax));
}

TEST(xnode_tests_json, out_of_safe_53bit_root_scalars_are_serialized_as_strings)
{
    EXPECT_EQ(xnode::ToJson(XValue(kJsonUnsafePosInt), nullptr, xnode::JsonFormat::kOneLine),
              Quote(std::to_string(kJsonUnsafePosInt)));
    EXPECT_EQ(xnode::ToJson(XValue(kJsonUnsafeNegInt), nullptr, xnode::JsonFormat::kOneLine),
              Quote(std::to_string(kJsonUnsafeNegInt)));
    EXPECT_EQ(xnode::ToJson(XValue(kJsonUnsafeUInt), nullptr, xnode::JsonFormat::kOneLine),
              Quote(std::to_string(kJsonUnsafeUInt)));
}

TEST(xnode_tests_json, safe_and_unsafe_53bit_values_are_serialized_with_expected_types)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    ASSERT_TRUE(node_map_sp);

    xnode::Set(node_map_sp, "a_safe_i64_max", kJsonSafeIntMax);
    xnode::Set(node_map_sp, "b_safe_i64_min", kJsonSafeIntMin);
    xnode::Set(node_map_sp, "c_safe_u64_max", kJsonSafeUIntMax);
    xnode::Set(node_map_sp, "d_unsafe_i64_pos", kJsonUnsafePosInt);
    xnode::Set(node_map_sp, "e_unsafe_i64_neg", kJsonUnsafeNegInt);
    xnode::Set(node_map_sp, "f_unsafe_u64", kJsonUnsafeUInt);
    xnode::Set(node_map_sp, "g_double", static_cast<double>(kJsonUnsafeUInt));

    const auto json     = xnode::ToJson(node_map_sp, nullptr, xnode::JsonFormat::kOneLine);
    const auto expected = std::string {"{"} + "\"a_safe_i64_max\":" + std::to_string(kJsonSafeIntMax) + "," +
                          "\"b_safe_i64_min\":" + std::to_string(kJsonSafeIntMin) + "," +
                          "\"c_safe_u64_max\":" + std::to_string(kJsonSafeUIntMax) + "," +
                          "\"d_unsafe_i64_pos\":" + Quote(std::to_string(kJsonUnsafePosInt)) + "," +
                          "\"e_unsafe_i64_neg\":" + Quote(std::to_string(kJsonUnsafeNegInt)) + "," +
                          "\"f_unsafe_u64\":" + Quote(std::to_string(kJsonUnsafeUInt)) + "," +
                          "\"g_double\":9007199254740992.0"
                          "}";

    EXPECT_EQ(json, expected);
}

TEST(xnode_tests_json, safe_53bit_numbers_roundtrip_without_type_loss)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    ASSERT_TRUE(node_map_sp);

    xnode::Set(node_map_sp, "a_i64_max", kJsonSafeIntMax);
    xnode::Set(node_map_sp, "b_i64_min", kJsonSafeIntMin);
    xnode::Set(node_map_sp, "c_u64_max", kJsonSafeUIntMax);

    const auto json                  = xnode::ToJson(node_map_sp, nullptr, xnode::JsonFormat::kOneLine);
    const auto [node_check, err_pos] = xnode::FromJson(json);
    ASSERT_TRUE(node_check);
    EXPECT_EQ(err_pos, 0U);

    size_t zDiff = 0;
    xnode::Compare(node_map_sp,
                   node_check,
                   true,
                   [&](const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&) {
                       ++zDiff;
                       return false;
                   });

    EXPECT_EQ(zDiff, 0U);
}

TEST(xnode_tests_json, out_of_safe_53bit_numbers_roundtrip_as_strings)
{
    auto node_map_sp = xnode::Create(INode::NodeType::Map);
    ASSERT_TRUE(node_map_sp);

    xnode::Set(node_map_sp, "a_i64_pos", kJsonUnsafePosInt);
    xnode::Set(node_map_sp, "b_i64_neg", kJsonUnsafeNegInt);
    xnode::Set(node_map_sp, "c_u64", kJsonUnsafeUInt);

    const auto json     = xnode::ToJson(node_map_sp, nullptr, xnode::JsonFormat::kOneLine);
    const auto expected = std::string {"{"} + "\"a_i64_pos\":" + Quote(std::to_string(kJsonUnsafePosInt)) + "," +
                          "\"b_i64_neg\":" + Quote(std::to_string(kJsonUnsafeNegInt)) + "," +
                          "\"c_u64\":" + Quote(std::to_string(kJsonUnsafeUInt)) + "}";

    EXPECT_EQ(json, expected);

    const auto [node_check, err_pos] = xnode::FromJson(json);
    ASSERT_TRUE(node_check);
    EXPECT_EQ(err_pos, 0U);

    EXPECT_EQ(xnode::ToJson(node_check, nullptr, xnode::JsonFormat::kOneLine), expected);
}

TEST(xnode_tests_json, array_respects_53bit_safe_integer_limits)
{
    auto node_array = xnode::Create(INode::NodeType::Array);
    ASSERT_TRUE(node_array);

    node_array->Insert(xnode::kIdxEnd, kJsonSafeIntMin);
    node_array->Insert(xnode::kIdxEnd, kJsonUnsafeNegInt);
    node_array->Insert(xnode::kIdxEnd, kJsonSafeUIntMax);
    node_array->Insert(xnode::kIdxEnd, kJsonUnsafeUInt);

    const auto json     = xnode::ToJson(node_array, nullptr, xnode::JsonFormat::kOneLine);
    const auto expected = std::string {"["} + std::to_string(kJsonSafeIntMin) + "," +
                          Quote(std::to_string(kJsonUnsafeNegInt)) + "," + std::to_string(kJsonSafeUIntMax) + "," +
                          Quote(std::to_string(kJsonUnsafeUInt)) + "]";

    EXPECT_EQ(json, expected);
}

TEST(xnode_tests_json, nested_map_respects_53bit_safe_integer_limits)
{
    auto root = xnode::Create(INode::NodeType::Map);
    ASSERT_TRUE(root);

    auto nested = xnode::Create(INode::NodeType::Map);
    ASSERT_TRUE(nested);

    xnode::Set(nested, "a_safe", kJsonSafeIntMax);
    xnode::Set(nested, "b_unsafe", kJsonUnsafePosInt);
    xnode::Set(nested, "c_unsafe_u", kJsonUnsafeUInt);

    xnode::Set(root, "nested", nested);

    const auto json = xnode::ToJson(root, nullptr, xnode::JsonFormat::kOneLine);
    const auto expected = std::string {
        "{"
        "\"nested\":{"
        "\"a_safe\":" +
        std::to_string(kJsonSafeIntMax) +
        ","
        "\"b_unsafe\":" +
        Quote(std::to_string(kJsonUnsafePosInt)) +
        ","
        "\"c_unsafe_u\":" +
        Quote(std::to_string(kJsonUnsafeUInt)) +
        "}"
        "}"};

    EXPECT_EQ(json, expected);
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

} // namespace xsdk::xnode::json_test
