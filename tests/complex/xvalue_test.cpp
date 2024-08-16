#include "xvalue/xvalue_rt.h"

#include "xbase.h"
#include "xobject_demo/xobject_demo.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <deque>
#include <thread>

using namespace xsdk;

double rand_d() { return (double)rand() / RAND_MAX; }

int32_t rand_int(double _dblMaxVal)
{
    uint32_t unMaxVal = (uint32_t)(std::abs(_dblMaxVal) + 0.5);
    if (_dblMaxVal < 0)
        return (int32_t)(rand_d() * unMaxVal * 2.0 - unMaxVal + 0.5);

    return (int32_t)(rand_d() * unMaxVal + 0.5);
}

// NOLINTBEGIN(*)

TEST(xobjects_tests, uids)
{
    constexpr auto res  = xbase::TypeUid<XValue>();
    constexpr auto res1 = xbase::TypeUid<IObject>();
    constexpr auto res2 = xbase::TypeUid<const IObject>();
    EXPECT_NE(res1, res);
    EXPECT_NE(res1, res2);
}

TEST(xvalue_tests, construct)
{
    {
        XValue xTest;
        EXPECT_EQ(xTest.Type(), XValue::kEmpty);
        EXPECT_FALSE(xTest);
    }
    //{
    //    XValue xTest(XValue::error());
    //    EXPECT_EQ( xTest.type(), XValue::xv_error);
    //    EXPECT_EQ(xTest, XValue::error());
    //    EXPECT_FALSE(xTest);
    //}
    {
        XValue xTest(nullptr);
        EXPECT_EQ(xTest.Type(), XValue::kNull);
        EXPECT_TRUE(xTest);
        EXPECT_TRUE(xTest.IsEmpty());
    }
    {
        double dblFake;
        XValue xTest(&dblFake);
        EXPECT_EQ(xTest.Type(), XValue::kNull);
        EXPECT_TRUE(xTest);
        EXPECT_TRUE(xTest.IsEmpty());
    }

    {
        XValue xTest(true);
        EXPECT_EQ(xTest.Type(), XValue::kBool);
        EXPECT_EQ(xTest.Int64(-1), 1);
        EXPECT_EQ(xTest.Uint64(99), 1);
    }
    {
        XValue xTest(false);
        EXPECT_EQ(xTest.Type(), XValue::kBool);
        EXPECT_EQ(xTest.Int64(-1), 0);
        EXPECT_EQ(xTest.Uint64(99), 0);
    }

    {
        XValue xTest(0);
        EXPECT_EQ(xTest.Type(), XValue::kInt64);
        EXPECT_EQ(xTest.Int64(-1), 0);
        EXPECT_EQ(xTest.Uint64(99), 0);
        EXPECT_FALSE(xTest.IsEmpty());
    }

    {
        XValue xTest((size_t)17);
        EXPECT_EQ(xTest.Type(), XValue::kUint64);
        EXPECT_EQ(xTest.Int64(-1), 17);
        EXPECT_EQ(xTest.Uint64(99), 17);
    }
    {
        XValue xTest((uint32_t)999);
        EXPECT_EQ(xTest.Type(), XValue::kUint64);
        EXPECT_EQ(xTest.Double(-1), 999.0);
        EXPECT_EQ(xTest.Int64(-1), 999);
        EXPECT_EQ(xTest.Uint64(99), 999);
    }
    {
        XValue xTest((int64_t)-777);
        EXPECT_EQ(xTest.Type(), XValue::kInt64);
        EXPECT_EQ(xTest.Double(-1), -777.0);
        EXPECT_EQ(xTest.Int64(-1), -777);
        EXPECT_EQ(xTest.Uint64(99, 100), 100);
    }

    {
        XValue xTest(-777.1);
        EXPECT_EQ(xTest.Type(), XValue::kDouble);
        EXPECT_EQ(xTest.Double(-1), -777.1);
        EXPECT_EQ(xTest.Int64(-1), -777);
        EXPECT_EQ(xTest.Uint64(99, 100), 100);
    }
    {
        XValue xTest(-777.1f);
        EXPECT_EQ(xTest.Type(), XValue::kDouble);
        EXPECT_EQ(xTest.Double(-1), -777.1f);
        EXPECT_EQ(xTest.Int64(-1), -777);
        EXPECT_EQ(xTest.Uint64(99, 100), 100);
    }
    {
        XValue xTest("string");
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
    }
    {
        XValue xTest(std::string("string"));
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
    }
    {
        std::string str("string");
        XValue      xTest(str);
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
    }
    {
        std::string str("string");
        XValue      xTest(str.c_str());
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
    }

    {
        std::string_view str("string");
        XValue           xTest(str);
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
    }

    {
        XValue xTest(std::string_view("string"));
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "string");
        EXPECT_EQ(std::string(xTest.StringView().data()), "string");
        EXPECT_FALSE(xTest.IsEmpty());
    }

    {
        XValue xTest(std::string_view(""));
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "");
        EXPECT_EQ(std::string(xTest.StringView().data()), "");
        EXPECT_TRUE(xTest.IsEmpty());
    }

    {
        const char* psz = nullptr;
        XValue      xTest(psz);
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "");
        EXPECT_EQ(std::string(xTest.StringView().data()), "");
        EXPECT_TRUE(xTest.IsEmpty());
    }

    {
        const auto* psz = "TEST";
        XValue      xTest((char*)psz);
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "TEST");
        EXPECT_EQ(std::string(xTest.StringView().data()), "TEST");
        EXPECT_FALSE(xTest.IsEmpty());
    }

    {
        char szTest[16] = {};
        strcpy(szTest, "TEST");
        XValue xTest(szTest);
        EXPECT_EQ(xTest.Type(), XValue::kString);
        EXPECT_EQ(xTest.String(), "TEST");
        EXPECT_EQ(std::string(xTest.StringView().data()), "TEST");
        EXPECT_FALSE(xTest.IsEmpty());
    }

    XValue obj(xobject::CreateShared());
    XValue obj_c(std::static_pointer_cast<const IObject>(xobject::CreateShared()));

    IObject::SPtr spObj = xobject::CreateShared();
}

TEST(xvalue_tests, compare)
{
    {
        XValue test1("1234");
        XValue test2("1234");
        XValue test3 = test1;
        XValue test33;
        test33 = test3;

        XValue test4;
        EXPECT_FALSE(test4);
        XValue test5(99.0);
        XValue test6(nullptr);

        EXPECT_EQ(test1.StringView().data(), test3.StringView().data());
        EXPECT_EQ(test1.StringView().data(), test33.StringView().data());

        EXPECT_TRUE(test1 == test2);
        EXPECT_TRUE(test3 == test2);
        EXPECT_TRUE(test3 == test1);
        EXPECT_TRUE(test4 != test2);
        EXPECT_TRUE(test5 != test2);
        EXPECT_TRUE(test6 != test2);

        EXPECT_EQ(test1, test2);
        EXPECT_EQ(test3, test2);
        EXPECT_NE(test4, test2);
        EXPECT_NE(test5, test2);
        EXPECT_NE(test6, test2);
    }

    {
        XValue test1("");
        XValue test2("");
        XValue test3 = test1;

        XValue test4;
        XValue test5(99.0);
        XValue test6(nullptr);

        EXPECT_EQ(test1, test2);
        EXPECT_EQ(test3, test2);
        EXPECT_NE(test4, test2);
        EXPECT_NE(test5, test2);
        EXPECT_NE(test6, test2);
    }
}

TEST(xvalue_tests, null_value)
{
    auto szXV   = sizeof(XValue);
    auto szStr  = sizeof(std::string);
    auto szStr1 = sizeof(std::unique_ptr<std::string>);
    auto szStr2 = sizeof(std::optional<std::string>);
    auto szStr3 = sizeof(std::shared_ptr<std::string>);

    auto szXV2 = sizeof(std::variant<std::monostate, bool, uint8_t, int32_t>);

    auto           spObj1   = xobject::CreateShared();
    IObject::SPtr  spObj2   = xobject::CreateShared();
    auto           spObj3   = xobject::CreateShared();
    IObject::SPtr  spObj4   = xobject::CreateShared();
    auto           spObj1_C = std::static_pointer_cast<const IObject>(xobject::CreateShared());
    IObject::SPtrC spObj2_C = std::static_pointer_cast<const IObject>(xobject::CreateShared());
    auto           spObj3_C = std::static_pointer_cast<const IObject>(xobject::CreateShared());
    IObject::SPtrC spObj4_C = std::static_pointer_cast<const IObject>(xobject::CreateShared());

    XValue chk1(spObj1);
    EXPECT_EQ(chk1.Type(), XValue::kObject);
    XValue chk2(spObj2);
    EXPECT_EQ(chk2.Type(), XValue::kObject);
    XValue chk3(std::move(spObj3));
    EXPECT_EQ(chk3.Type(), XValue::kObject);
    spObj3.reset(); // seems gcc do not move static_pointer_cast(U&&...)) ?
    EXPECT_FALSE(spObj3);
    XValue chk4(std::move(spObj4));
    EXPECT_EQ(chk4.Type(), XValue::kObject);
    EXPECT_FALSE(spObj4);

    XValue chk1_c(spObj1_C);
    EXPECT_EQ(chk1_c.Type(), XValue::kConstObject);
    XValue chk2_c(spObj2_C);
    EXPECT_EQ(chk2_c.Type(), XValue::kConstObject);
    XValue chk3_c(std::move(spObj3_C));
    EXPECT_EQ(chk3_c.Type(), XValue::kConstObject);
    spObj3_C.reset(); // seems gcc do not move static_pointer_cast(U&&...) ?
    EXPECT_FALSE(spObj3_C);
    XValue chk4_c(std::move(spObj4_C));
    EXPECT_EQ(chk4_c.Type(), XValue::kConstObject);
    EXPECT_FALSE(spObj4_C);

    XValue chk3_null(spObj3);
    XValue chk4_null(spObj4);
    XValue chk3_null_c(spObj3_C);
    XValue chk4_null_c(spObj4_C);

    EXPECT_EQ(chk3_null.Type(), XValue::kNull);
    EXPECT_EQ(chk4_null.Type(), XValue::kNull);
    EXPECT_EQ(chk3_null_c.Type(), XValue::kNull);
    EXPECT_EQ(chk4_null_c.Type(), XValue::kNull);

    chk1 = spObj3;
    EXPECT_EQ(chk1.Type(), XValue::kNull);
    chk2 = spObj3_C;
    EXPECT_EQ(chk2.Type(), XValue::kNull);

    chk1 = std::move(spObj3);
    EXPECT_EQ(chk1.Type(), XValue::kNull);
    chk2 = std::move(spObj3_C);
    EXPECT_EQ(chk2.Type(), XValue::kNull);

    {
        XValue chk3_null(std::move(spObj3));
        XValue chk4_null(std::move(spObj4));
        XValue chk3_null_c(std::move(spObj3_C));
        XValue chk4_null_c(std::move(spObj4_C));

        EXPECT_EQ(chk3_null.Type(), XValue::kNull);
        EXPECT_EQ(chk4_null.Type(), XValue::kNull);
        EXPECT_EQ(chk3_null_c.Type(), XValue::kNull);
        EXPECT_EQ(chk4_null_c.Type(), XValue::kNull);
    }
}

TEST(xvalue_tests, rt_test)
{
    using XValueRT = XTimed<XValue, xnode::UniqueClock<>>;

    XValueRT val_with_ts(100);
    XValueRT val_no_ts;
    auto     t = val_with_ts.Timestamp();
    EXPECT_GT(t, 0);
    EXPECT_FALSE(val_with_ts.TimeIsAbsent());
    EXPECT_TRUE(val_no_ts.TimeIsAbsent());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto ts = val_with_ts.TimeElapsed();
    EXPECT_GT(ts, XValueRT::MsecToTicks(99));
    EXPECT_LT(ts, XValueRT::MsecToTicks(150));

    auto ts_no = val_no_ts.TimeElapsed();
    EXPECT_EQ(ts_no, 0);

    EXPECT_EQ(val_no_ts.Timestamp(), kAbsentRT);
}

// TEST(modern_tests, perf_clock)
//{
//     xbase::clock_cpp clockTest;
//
// #ifdef _DEBUG
//     size_t zTests = 10'000;
// #else
//     size_t zTests             = 1'000'000;
// #endif
//     {
//         std::atomic_int64_t llVal = {};
//         std::deque<int64_t> deqTest;
//         for (size_t z = 0; z < zTests; ++z) {
//             deqTest.push_back(Monotonic<int64_t>::Next(z % 100));
//             // deqTest.push_back(z);
//             if (deqTest.size() > 1000)
//                 deqTest.pop_front();
//         }
//
//         double ppms = zTests / clockTest.GetStepMsec(false);
//         std::cout // << _zTests << " " << typeid(TMap).name() << std::endl
//             << " mod @ msec:" << clockTest.GetStepMsec(true) << "  ppms:" << ppms << std::endl;
//     }
//     {
//
//         std::deque<int64_t> deqTest;
//         for (size_t z = 0; z < zTests; ++z) {
//             auto t = UniqueClock<std::chrono::high_resolution_clock>::Timestamp();
//             deqTest.push_back(t);
//             if (deqTest.size() > 1000)
//                 deqTest.erase(deqTest.begin());
//         }
//
//         double ppms = zTests / clockTest.GetStepMsec(false);
//         std::cout // << _zTests << " " << typeid(TMap).name() << std::endl
//             << " UniqueClock @ msec:" << clockTest.GetStepMsec(true) << "  ppms:" << ppms << std::endl;
//     }
//
//     // EXPECT_FALSE(1);
// }

TEST(modern_tests, monotonic_test)
{

    size_t zTests = 1'000'000;
    {
        std::vector<std::thread> vecThreads;
        for (size_t z = 0; z < 10; ++z) {
            auto th = std::thread([zTests]() {
                int64_t prev = std::numeric_limits<int64_t>::min();
                for (size_t z = 0; z < zTests; ++z) {
                    auto v = xnode::Monotonic<char>::Next(rand_int((double)zTests * 10 * z));
                    ASSERT_GT(v, prev);
                    prev = v;
                }
            });

            vecThreads.push_back(std::move(th));
        }

        for (auto& th : vecThreads)
            th.join();
    }

    auto val = xnode::Monotonic<int>::Next(0);
    EXPECT_EQ(val, 0);
    val = xnode::Monotonic<int>::Next(0);
    EXPECT_EQ(val, 1);
    auto v1 = xnode::Monotonic<int, 10>::Next(0);
    EXPECT_EQ(v1, 0);
    v1 = xnode::Monotonic<int, 10>::Next(0);
    EXPECT_EQ(v1, 1);
    v1 = xnode::Monotonic<int, 11>::Next(0);
    EXPECT_EQ(v1, 0);
    v1 = xnode::Monotonic<int, 11>::Next(0);
    EXPECT_EQ(v1, 1);
}

// NOLINTEND(*)