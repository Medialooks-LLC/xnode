#include "xvalue/xvalue.h"

#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

namespace xsdk {
namespace xvalue_tests {

    TEST(XValueFromStringTest, ParsesTrueCaseInsensitive)
    {
        const auto value  = XValue::FromString("TrUe");
        EXPECT_EQ(value.Type(), XValue::kBool);
        const auto parsed = value.OptionalGet<bool>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_TRUE(parsed.value());
    }

    TEST(XValueFromStringTest, ParsesFalseCaseInsensitive)
    {
        const auto value  = XValue::FromString("fAlSe");
        EXPECT_EQ(value.Type(), XValue::kBool);
        const auto parsed = value.OptionalGet<bool>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_FALSE(parsed.value());
    }

    TEST(XValueFromStringTest, ParsesSignedDecimalInt)
    {
        const auto value  = XValue::FromString("-10");
        const auto parsed = value.OptionalGet<int64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), -10);
    }

    TEST(XValueFromStringTest, ParsesUnsignedDecimalAsInt64WhenFits)
    {
        const auto value  = XValue::FromString("0010");
        const auto parsed = value.OptionalGet<int64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), 10);
    }

    TEST(XValueFromStringTest, ParsesHexInteger)
    {
        const auto value  = XValue::FromString("0x10");
        EXPECT_EQ(value.Type(), XValue::kUint64);
        const auto parsed = value.OptionalGet<uint64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), 16);
    }

    TEST(XValueFromStringTest, ParsesUint64WhenAboveInt64Max)
    {
        const auto value  = XValue::FromString("18446744073709551615");
        EXPECT_EQ(value.Type(), XValue::kUint64);
        const auto parsed = value.OptionalGet<uint64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), std::numeric_limits<uint64_t>::max());
    }

    TEST(XValueFromStringTest, ParsesPlusUint64WhenAboveInt64Max)
    {
        const auto value  = XValue::FromString("+18446744073709551615");
        EXPECT_EQ(value.Type(), XValue::kUint64);
        const auto parsed = value.OptionalGet<uint64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), std::numeric_limits<uint64_t>::max());
    }

    TEST(XValueFromStringTest, ParsesDouble)
    {
        const auto value  = XValue::FromString("010.5");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), 10.5);
    }

    TEST(XValueFromStringTest, ParsesDoubleDotFirst)
    {
        const auto value  = XValue::FromString(".77");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), 0.77);
    }

    TEST(XValueFromStringTest, ParsesDoubleNeg)
    {
        const auto value = XValue::FromString("-10.5");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), -10.5);
    }

    TEST(XValueFromStringTest, ParsesDoubleNegDotFirst)
    {
        const auto value = XValue::FromString(" -.77");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), -0.77);
    }
    TEST(XValueFromStringTest, ParsesDoublePlus)
    {
        const auto value = XValue::FromString("+10.5");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), 10.5);
    }

    TEST(XValueFromStringTest, ParsesDoublePlusDotFirst)
    {
        const auto value = XValue::FromString("+.77");
        EXPECT_EQ(value.Type(), XValue::kDouble);
        const auto parsed = value.OptionalGet<double>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_DOUBLE_EQ(parsed.value(), 0.77);
    }
    TEST(XValueFromStringTest, KeepsEmptyStringAsString)
    {
        const auto value  = XValue::FromString("");
        const auto parsed = value.OptionalGet<std::string_view>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_TRUE(parsed.value().empty());
    }

    TEST(XValueFromStringTest, KeepsAlphaNumericStringAsString)
    {
        const auto value  = XValue::FromString("10M");
        const auto parsed = value.OptionalGet<std::string_view>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), "10M");
    }

    TEST(XValueFromStringTest, KeepsTextAsString)
    {
        const auto value  = XValue::FromString("abc");
        const auto parsed = value.OptionalGet<std::string_view>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), "abc");
    }

    TEST(XValueFromStringTest, ParsesSignedValueWithPlus)
    {
        const auto value  = XValue::FromString("+10");
        const auto parsed = value.OptionalGet<int64_t>();

        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), 10);
    }

} // namespace
} // namespace xsdk
