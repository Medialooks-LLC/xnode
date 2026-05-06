#include "xnode.h"

#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

namespace xsdk {
namespace xnode_tests {

    TEST(ParseParamStringToNodeTest, EmptyInputReturnsNullNodeAndNoErrors)
    {
        const auto [node, parse_res] = xnode::ParseParamString("", false);

        EXPECT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());
        EXPECT_TRUE(parse_res.StorageShared());
    }

    TEST(ParseParamStringToNodeTest, ParsesTypedUnquotedValues)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a=10 b=0x77 c=0x10 d=10.5 e=true f=false s=10M", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto a = xnode::At(node, XPath("a"));
        const auto b = xnode::At(node, XPath("b"));
        const auto c = xnode::At(node, XPath("c"));
        const auto d = xnode::At(node, XPath("d"));
        const auto e = xnode::At(node, XPath("e"));
        const auto f = xnode::At(node, XPath("f"));
        const auto s = xnode::At(node, XPath("s"));

        ASSERT_TRUE(a);
        ASSERT_TRUE(b);
        ASSERT_TRUE(c);
        ASSERT_TRUE(d);
        ASSERT_TRUE(e);
        ASSERT_TRUE(f);
        ASSERT_TRUE(s);

        const auto a_v = a.template OptionalGet<int64_t>();
        const auto b_v = b.template OptionalGet<int64_t>();
        const auto c_v = c.template OptionalGet<int64_t>();
        const auto d_v = d.template OptionalGet<double>();
        const auto e_v = e.template OptionalGet<bool>();
        const auto f_v = f.template OptionalGet<bool>();
        const auto s_v = s.template OptionalGet<std::string_view>();

        ASSERT_TRUE(a_v.has_value());
        ASSERT_TRUE(b_v.has_value());
        ASSERT_TRUE(c_v.has_value());
        ASSERT_TRUE(d_v.has_value());
        ASSERT_TRUE(e_v.has_value());
        ASSERT_TRUE(f_v.has_value());
        ASSERT_TRUE(s_v.has_value());

        EXPECT_EQ(a_v.value(), 10);
        EXPECT_EQ(b_v.value(), 0x77);
        EXPECT_EQ(c_v.value(), 16);
        EXPECT_DOUBLE_EQ(d_v.value(), 10.5);
        EXPECT_TRUE(e_v.value());
        EXPECT_FALSE(f_v.value());
        EXPECT_EQ(s_v.value(), "10M");
    }

    TEST(ParseParamStringToNodeTest, KeepsQuotedValuesAsStrings)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a='10' b=\"true\" c='10.5' d='077' e='0x10'", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto a = xnode::At(node, XPath("a"));
        const auto b = xnode::At(node, XPath("b"));
        const auto c = xnode::At(node, XPath("c"));
        const auto d = xnode::At(node, XPath("d"));
        const auto e = xnode::At(node, XPath("e"));

        ASSERT_TRUE(a);
        ASSERT_TRUE(b);
        ASSERT_TRUE(c);
        ASSERT_TRUE(d);
        ASSERT_TRUE(e);

        const auto a_v = a.template OptionalGet<std::string_view>();
        const auto b_v = b.template OptionalGet<std::string_view>();
        const auto c_v = c.template OptionalGet<std::string_view>();
        const auto d_v = d.template OptionalGet<std::string_view>();
        const auto e_v = e.template OptionalGet<std::string_view>();

        ASSERT_TRUE(a_v.has_value());
        ASSERT_TRUE(b_v.has_value());
        ASSERT_TRUE(c_v.has_value());
        ASSERT_TRUE(d_v.has_value());
        ASSERT_TRUE(e_v.has_value());

        EXPECT_EQ(a_v.value(), "10");
        EXPECT_EQ(b_v.value(), "true");
        EXPECT_EQ(c_v.value(), "10.5");
        EXPECT_EQ(d_v.value(), "077");
        EXPECT_EQ(e_v.value(), "0x10");
    }

    TEST(ParseParamStringToNodeTest, KeepsAllValuesAsStringsWhenRequested)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a=10 b=true c=10.5 d=077 e=0x10", true);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto a = xnode::At(node, XPath("a"));
        const auto b = xnode::At(node, XPath("b"));
        const auto c = xnode::At(node, XPath("c"));
        const auto d = xnode::At(node, XPath("d"));
        const auto e = xnode::At(node, XPath("e"));

        ASSERT_TRUE(a);
        ASSERT_TRUE(b);
        ASSERT_TRUE(c);
        ASSERT_TRUE(d);
        ASSERT_TRUE(e);

        const auto a_v = a.template OptionalGet<std::string_view>();
        const auto b_v = b.template OptionalGet<std::string_view>();
        const auto c_v = c.template OptionalGet<std::string_view>();
        const auto d_v = d.template OptionalGet<std::string_view>();
        const auto e_v = e.template OptionalGet<std::string_view>();

        ASSERT_TRUE(a_v.has_value());
        ASSERT_TRUE(b_v.has_value());
        ASSERT_TRUE(c_v.has_value());
        ASSERT_TRUE(d_v.has_value());
        ASSERT_TRUE(e_v.has_value());

        EXPECT_EQ(a_v.value(), "10");
        EXPECT_EQ(b_v.value(), "true");
        EXPECT_EQ(c_v.value(), "10.5");
        EXPECT_EQ(d_v.value(), "077");
        EXPECT_EQ(e_v.value(), "0x10");
    }

    TEST(ParseParamStringToNodeTest, IgnoresFlagsWhenValueForFlagsIsEmpty)
    {
        const auto [node, parse_res] = xnode::ParseParamString("flag1 flag2 a=10", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto flag1 = xnode::At(node, XPath("flag1"));
        const auto flag2 = xnode::At(node, XPath("flag2"));
        const auto a     = xnode::At(node, XPath("a"));

        EXPECT_FALSE(flag1);
        EXPECT_FALSE(flag2);
        ASSERT_TRUE(a);

        const auto a_v = a.template OptionalGet<int64_t>();
        ASSERT_TRUE(a_v.has_value());
        EXPECT_EQ(a_v.value(), 10);
    }

    TEST(ParseParamStringToNodeTest, StoresFlagsWhenValueForFlagsProvided)
    {
        const auto [node, parse_res] = xnode::ParseParamString("flag1 flag2 a=10", false, XValue(true));

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto flag1 = xnode::At(node, XPath("flag1"));
        const auto flag2 = xnode::At(node, XPath("flag2"));
        const auto a     = xnode::At(node, XPath("a"));

        ASSERT_TRUE(flag1);
        ASSERT_TRUE(flag2);
        ASSERT_TRUE(a);

        const auto flag1_v = flag1.template OptionalGet<bool>();
        const auto flag2_v = flag2.template OptionalGet<bool>();
        const auto a_v     = a.template OptionalGet<int64_t>();

        ASSERT_TRUE(flag1_v.has_value());
        ASSERT_TRUE(flag2_v.has_value());
        ASSERT_TRUE(a_v.has_value());

        EXPECT_TRUE(flag1_v.value());
        EXPECT_TRUE(flag2_v.value());
        EXPECT_EQ(a_v.value(), 10);
    }

    TEST(ParseParamStringToNodeTest, SupportsNestedPaths)
    {
        const auto [node, parse_res] = xnode::ParseParamString("video::codec='q264sw' video::b=10 audio::enabled=true", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto codec   = xnode::At(node, XPath("video::codec"));
        const auto bitrate = xnode::At(node, XPath("video::b"));
        const auto enabled = xnode::At(node, XPath("audio::enabled"));

        ASSERT_TRUE(codec);
        ASSERT_TRUE(bitrate);
        ASSERT_TRUE(enabled);

        const auto codec_v   = codec.template OptionalGet<std::string_view>();
        const auto bitrate_v = bitrate.template OptionalGet<int64_t>();
        const auto enabled_v = enabled.template OptionalGet<bool>();

        ASSERT_TRUE(codec_v.has_value());
        ASSERT_TRUE(bitrate_v.has_value());
        ASSERT_TRUE(enabled_v.has_value());

        EXPECT_EQ(codec_v.value(), "q264sw");
        EXPECT_EQ(bitrate_v.value(), 10);
        EXPECT_TRUE(enabled_v.value());
    }

    TEST(ParseParamStringToNodeTest, PreservesErrorViewsByOwningStorage)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a='abc", false);

        EXPECT_TRUE(node);
        ASSERT_TRUE(parse_res.StorageShared());
        ASSERT_EQ(parse_res.errors.size(), 1U);

        EXPECT_EQ(parse_res.errors[0].code, xbase::ParamParseErrorCode::kUnterminatedQuotedValue);
        EXPECT_EQ(parse_res.errors[0].position, 2U);
        EXPECT_EQ(parse_res.errors[0].key.text, "a");
        EXPECT_EQ(parse_res.errors[0].token.text, "'abc");
    }

    TEST(ParseParamStringToNodeTest, ReturnsErrorAndStillBuildsNodeWhenPossible)
    {
        const auto [node, parse_res] = xnode::ParseParamString("=abc good=1", false);

        ASSERT_TRUE(node);
        ASSERT_EQ(parse_res.errors.size(), 1U);

        EXPECT_EQ(parse_res.errors[0].code, xbase::ParamParseErrorCode::kEmptyKey);
        EXPECT_EQ(parse_res.errors[0].position, 0U);

        const auto good = xnode::At(node, XPath("good"));
        ASSERT_TRUE(good);

        const auto good_v = good.template OptionalGet<int64_t>();
        ASSERT_TRUE(good_v.has_value());
        EXPECT_EQ(good_v.value(), 1);
    }

    TEST(ParseParamStringToNodeTest, ParsesAdjacentTokenAfterQuotedValue)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a='x'b=1", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto a = xnode::At(node, XPath("a"));
        const auto b = xnode::At(node, XPath("b"));

        ASSERT_TRUE(a);
        ASSERT_TRUE(b);

        const auto a_v = a.template OptionalGet<std::string_view>();
        const auto b_v = b.template OptionalGet<int64_t>();

        ASSERT_TRUE(a_v.has_value());
        ASSERT_TRUE(b_v.has_value());

        EXPECT_EQ(a_v.value(), "x");
        EXPECT_EQ(b_v.value(), 1);
    }

    TEST(ParseParamStringToNodeTest, CreatesNestedNodeForParentChildStringPath)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::child='value'", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::child"), XValue(std::string_view("value"))},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, CreatesNestedNodeForParentChildTypedPath)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::child=10", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::child"), XValue(static_cast<int64_t>(10))},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, CreatesNestedNodeForSeveralParentChildPaths)
    {
        const auto [node, parse_res] = xnode::ParseParamString("video::codec='q264sw' video::b=10 audio::enabled=true", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("video::codec"), XValue(std::string_view("q264sw"))},
            {XPath("video::b"), XValue(static_cast<int64_t>(10))},
            {XPath("audio::enabled"), XValue(true)},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, CreatesNestedNodeAndFlagUnderParentPath)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::child=1 parent::enabled", false, XValue(true));

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::child"), XValue(static_cast<int64_t>(1))},
            {XPath("parent::enabled"), XValue(true)},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, KeepsQuotedNumericNestedValueAsString)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::child='10'", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::child"), XValue(std::string_view("10"))},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, KeepsAllNestedValuesAsStringsWhenRequested)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::int=10 parent::bool=true parent::double=10.5", true);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::int"), XValue(std::string_view("10"))},
            {XPath("parent::bool"), XValue(std::string_view("true"))},
            {XPath("parent::double"), XValue(std::string_view("10.5"))},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::ToJson(node), xnode::ToJson(expected));
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

    TEST(ParseParamStringToNodeTest, LatestValueOverwritesPreviousForSameKey)
    {
        const auto [node, parse_res] = xnode::ParseParamString("a=1 a=2", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        const auto a = xnode::At(node, XPath("a"));
        ASSERT_TRUE(a);

        const auto a_v = a.template OptionalGet<int64_t>();
        ASSERT_TRUE(a_v.has_value());
        EXPECT_EQ(a_v.value(), 2);
    }

    TEST(ParseParamStringToNodeTest, LatestValueOverwritesPreviousForSameNestedKey)
    {
        const auto [node, parse_res] = xnode::ParseParamString("parent::child=1 parent::child=2", false);

        ASSERT_TRUE(node);
        EXPECT_FALSE(parse_res.HasErrors());

        auto expected = xnode::CreateComplex({
            {XPath("parent::child"), XValue(static_cast<int64_t>(2))},
        });

        ASSERT_TRUE(expected);
        EXPECT_EQ(xnode::Compare(node, expected, true), 0);
    }

} // namespace
} // namespace xsdk::xnode