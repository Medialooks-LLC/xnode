#include "xkey/xkey.h"
#include "xvalue/xvalue.h"
#include "xobject_demo/xobject_demo.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <thread>

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xkey_tests, key_compare)
{
    std::vector<XKey> vec_keys = {99, "cde", 11, "abc"};
    std::sort(vec_keys.begin(), vec_keys.end());
    EXPECT_EQ(vec_keys[0], XKey(11));
    EXPECT_EQ(vec_keys[1], XKey(99));
    EXPECT_EQ(vec_keys[2], XKey("abc"));
    EXPECT_EQ(vec_keys[3], XKey("cde"));
}

TEST(xkey_tests, map_with_xkey)
{
    std::map<XKey, XValue> mapTest;

    mapTest[99]    = "vsevolod";
    mapTest["abc"] = 3.14;
    mapTest["cde"] = true;

    EXPECT_EQ(mapTest[99].StringView(), "vsevolod");
    EXPECT_EQ(mapTest["abc"].Double(), 3.14);
    EXPECT_EQ(mapTest["cde"].Bool(), true);
    EXPECT_EQ(mapTest["zde"].Bool(), false);

    EXPECT_TRUE(mapTest[17].StringView().empty());
}


// NOLINTEND(*)