#include "xnode.h"
#include "xnode_functions.h"
#include "xnode_json.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

#include "utils_temp.h"

using namespace xsdk;

// NOLINTBEGIN(*)

TEST(xnode_thread_tests, thread_values_ops)
{
    auto node_map_sp = xnode::CreateMap({{"x01", true}, {"x02", "str_xxx"}, {"x03", 99.0}}, "root");

    std::cout << "STARTED" << std::endl;

    std::atomic_bool stop = {false};

    size_t                   keys_count = 20;
    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (size_t z = 0; z < keys_count; ++z) {
        keys.push_back(xutils_temp::str_format("%02zd", z));
        values.push_back(xutils_temp::str_format("%02zd_val", z));
    }

    std::atomic<size_t> rounds    = 0;
    std::atomic<size_t> erased    = 0;
    std::atomic<size_t> chanaged  = 0;
    std::atomic<size_t> exchanged = 0;
    auto                pf_thread = [&]() {
        XValue prev;
        while (!stop) {

            size_t idx  = rand() % keys.size();
            size_t idx2 = rand() % keys.size();
            auto   key  = keys[idx];
            switch (rand() % 16) {
                case 0:
                    node_map_sp->Set(key, values[idx]);
                    break;
                case 1:
                    node_map_sp->Insert(key, values[idx2]);
                    break;
                case 3:
                    node_map_sp->Erase(rand() % keys.size());
                    break;
                case 4:
                    prev = node_map_sp->At(rand() % keys.size());
                    break;
                case 5:
                    exchanged += node_map_sp->CompareExchange(key, values[10], values[7]).first;
                    break;
                case 6:
                    erased += node_map_sp->BulkErase({keys[11], keys[idx], keys[idx2]}).size();
                    break;
                case 7:
                    node_map_sp->BulkSet(
                        {{keys[1], values[1]}, {keys[idx], values[2]}, {keys[idx2], values[3]}, {keys[4], values[4]}});
                    break;
                case 8:
                    node_map_sp->BulkInsert({{keys[10], values[1]},
                                              {keys[idx2], values[2]},
                                              {keys[idx], values[3]},
                                              {keys[14], values[4]}});
                    break;
                case 9:
                    chanaged += node_map_sp->KeyChange(keys[idx], keys[idx2]) ? 1 : 0;
                    break;
                case 10: {
                    auto vec_values = node_map_sp->BulkGetAll();
                    break;
                }
                case 12: {
                    auto node_new = xnode::NodeGet(node_map_sp, "nodetest::next", INode::NodeType::Map);
                    break;
                }
                case 13: {
                    auto node_new = xnode::NodeGet(node_map_sp, "nodetest::next2", INode::NodeType::Map);
                    break;
                }
                case 14: {
                    auto node_p = xnode::NodeGet(node_map_sp, "nodetest", INode::NodeType::Map);
                    if (node_p)
                        node_p->Erase(0);
                    break;
                }
                case 15: {
                    size_t idx = node_map_sp->Size();
                    node_map_sp->BulkErase({idx / 2, idx / 2 + 1, idx / 2 + 2});
                    break;
                }

            }

            rounds++;
        }
    };

    size_t                   threads = 8;
    std::vector<std::thread> vec_threads;
    for (size_t z = 0; z < threads; ++z) {
        vec_threads.emplace_back(pf_thread);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    stop = true;
    for (auto& th : vec_threads)
        th.join();

    std::cout << xnode::ToJson(node_map_sp) << std::endl;

    std::cout << "rounds:" << std::to_string(rounds.load()) << std::endl;
    std::cout << "erased:" << std::to_string(erased.load()) << std::endl;
    std::cout << "keys_chnaged:" << std::to_string(chanaged.load()) << std::endl;
    std::cout << "excanged:" << std::to_string(exchanged.load()) << std::endl;

    std::cout << "FINSIHED" << std::endl;

    // EXPECT_FALSE(TRUE);
}

TEST(xnode_thread_tests, thread_node_parents_ops)
{
#ifndef _DEBUG
    auto node_map_sp   = xnode::Create(INode::NodeType::Map, "root");
    auto node_array_sp = xnode::CreateArray({}, "array");

    INode::SPtr nodes[] = {xnode::CreateMap({{"name1", "01"}}, "01"),
                          xnode::CreateMap({{"name2", "02"}}, "02"),
                          xnode::CreateMap({{"name3", "03"}}, "03"),
                          xnode::CreateMap({{"name4", "04"}}, "04"),
                          xnode::CreateMap({{"name5", "05"}}, "05"),
                          xnode::CreateArray({"arr00", true, "0222"}, "arr_01"),
                          xnode::CreateArray({"arr01", true, "0222"}, "arr_01"),
                          xnode::CreateArray({"arr02", nullptr, "0222"}, "arr_02"),
                          xnode::CreateArray({"arr04", "0222"}, "arr_03")};

    std::atomic_bool stop = {false};

    std::atomic<size_t> rounds    = 0;
    auto                pf_thread = [&]() {
        XValue prev;
        while (!stop) {

            size_t idx1 = rand() % SIZEOF_ARRAY(nodes);
            size_t idx2 = rand() % SIZEOF_ARRAY(nodes);

            switch (rand() % 12) {
                case 0:
                    nodes[idx1]->ParentSet(node_map_sp);
                    break;
                case 1:
                    nodes[idx1]->ParentSet(node_array_sp);
                    break;
                case 3:
                    xnode::NodeInsert(node_map_sp, nodes[idx2], true);
                    break;
                case 4:
                    xnode::NodeInsert(node_array_sp, nodes[idx2], true);
                    break;
                case 5:
                    xnode::NodeInsert(nodes[idx1], nodes[idx2], true);
                    break;
                case 6:
                    xnode::NodeInsert(nodes[idx1], nodes[idx2], false);
                    break;
                case 7:
                    nodes[idx1]->ParentSet(nodes[idx2]);
                    break;
                case 8:
                    nodes[idx1]->ParentDetach();
                    break;
                case 9:
                    //node_map_sp->Clear();
                    break;
                case 10:
                    nodes[idx1]->NameSet("name_" + std::to_string(rand() % 10), true);
                    break;
                case 11:
                    nodes[idx1]->BulkErase({0, 2, 3});
                    break;
            }

            rounds++;
        }
    };

    for (size_t z = 0; z < SIZEOF_ARRAY(nodes); ++z)
        nodes[z]->ParentSet(node_map_sp);

    size_t                   threads = 8;
    std::vector<std::thread> vec_threads;
    for (size_t z = 0; z < threads; ++z) {
        vec_threads.emplace_back(pf_thread);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    stop = true;
    for (auto& th : vec_threads)
        th.join();

    std::cout << "rounds:" << std::to_string(rounds.load()) << std::endl;
    auto improper_map = xnode::ParentsCheck(node_map_sp, true);
    auto fixed_map    = xnode::ParentsFix(improper_map);
    std::cout << "IMPROPER FOUND:" << std::to_string(improper_map.size()) << std::endl;
    std::cout << "IMPROPER FIXED:" << std::to_string(fixed_map.size()) << std::endl;
    //std::cout << "MAP" << xnode::ToJson(node_map_sp) << std::endl;
    
    //std::cout << "ARRAY" << xnode::ToJson(node_array_sp) << std::endl;

    

    std::cout << "FINSIHED" << std::endl;
    //EXPECT_FALSE(TRUE);
#endif
}

TEST(xnode_thread_tests, thread_node_parents_ops2)
{
#ifndef _DEBUG
    auto node_map_sp   = xnode::Create(INode::NodeType::Map, "root");
    auto node_array_sp = xnode::CreateArray({}, "array");

    INode::SPtr nodes[] = {xnode::CreateMap({{"name1", "01"}}, "01"),
                          xnode::CreateMap({{"name2", "02"}}, "02"),
                          xnode::CreateMap({{"name3", "03"}}, "03"),
                          xnode::CreateMap({{"name4", "04"}}, "04"),
                          xnode::CreateMap({{"name5", "05"}}, "05"),
                          xnode::CreateArray({"arr01", true, "0222"}, "arr_01"),
                          xnode::CreateArray({"arr02", nullptr, "0222"}, "arr_02"),
                          xnode::CreateArray({"arr04", "0222"}, "arr_03")};

    std::atomic_bool stop = {false};

    std::atomic<size_t> rounds    = 0;
    auto                pf_thread = [&]() {
        XValue prev;
        while (!stop) {

            size_t idx1 = rand() % SIZEOF_ARRAY(nodes);
            size_t idx2 = rand() % SIZEOF_ARRAY(nodes);

            switch (rand() % 12) {
                case 0:
                    nodes[idx1]->ParentDetach();
                    nodes[idx1]->ParentSet(node_map_sp);
                    break;
                case 1:
                    nodes[idx1]->ParentDetach();
                    nodes[idx1]->ParentSet(node_array_sp);
                    break;
                case 3:
                    nodes[idx2]->ParentDetach();
                    xnode::NodeInsert(node_map_sp, nodes[idx2], true);
                    break;
                case 4:
                    nodes[idx2]->ParentDetach();
                    xnode::NodeInsert(node_array_sp, nodes[idx2], true);
                    break;
                case 5:
                    nodes[idx2]->ParentDetach();
                    xnode::NodeInsert(nodes[idx1], nodes[idx2], true);
                    break;
                case 6:
                    nodes[idx2]->ParentDetach();
                    xnode::NodeInsert(nodes[idx1], nodes[idx2], false);
                    break;
                case 7:
                    nodes[idx1]->ParentDetach();
                    nodes[idx1]->ParentSet(nodes[idx2]);
                    break;
                case 8:
                    nodes[idx1]->ParentDetach();
                    nodes[idx1]->ParentSet(nodes[idx2]);
                    break;
                case 9:
                    // node_map_sp->Clear();
                    break;
                case 10:
                    // node_array_sp->Clear();
                    break;
                case 11:
                    nodes[idx1]->BulkErase({2, 3});
                    break;
            }

            rounds++;
        }
    };

    for (size_t z = 0; z < SIZEOF_ARRAY(nodes); ++z)
        nodes[z]->ParentSet(node_map_sp);

    size_t                   threads = 8;
    std::vector<std::thread> vec_threads;
    for (size_t z = 0; z < threads; ++z) {
        vec_threads.emplace_back(pf_thread);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    stop = true;
    for (auto& th : vec_threads)
        th.join();

    //std::cout << "MAP" << xnode::ToJson(node_map_sp) << std::endl;

    //std::cout << "ARRAY" << xnode::ToJson(node_array_sp) << std::endl;

    std::cout << "rounds:" << std::to_string(rounds.load()) << std::endl;

    std::cout << "FINISHED" << std::endl;
    // EXPECT_FALSE(TRUE);
#endif
}
TEST(xnode_thread_tests, thread_node_complex_ops)
{
    auto node_map_sp   = xnode::Create(INode::NodeType::Map, "root");

    size_t              node_names     = 10;
    size_t              val_names = 30;
    size_t              depth     = 3;
    std::atomic_bool stop = {false};
    std::atomic<size_t> rounds    = 0;
    auto                pf_thread = [&]() {
        XValue prev;
        while (!stop) {

            std::vector<std::string> keys(depth);
            for (auto& key : keys)
                key = xutils_temp::str_format("%03d", rand() % node_names);

            auto value_name = xutils_temp::str_format("val_%03d", rand() % val_names);
            switch (rand() % 9) {
                case 0: {
                    xnode::Set(node_map_sp, XPath(keys[0], value_name), "first");
                    auto val = xnode::At(node_map_sp, XPath(keys[0], keys[1]));
                    break;
                }
                case 1: {
                    xnode::Set(node_map_sp, XPath(keys[0], keys[1], value_name), "second");
                    auto val = xnode::At(node_map_sp, XPath(keys[0], keys[1], keys[2]));
                    break;
                }
                case 2: {
                    xnode::Set(node_map_sp, XPath(keys[0], keys[1], keys[2], value_name), "third");
                    auto val = xnode::At(node_map_sp, XPath(keys[1], keys[0], keys[2]));
                    break;
                }
                case 3:
                    //xnode_erase(node_map_sp, XPath(keys[0]));
                    break;
                case 4:
                    xnode::Erase(node_map_sp, XPath(keys[0], keys[1]));
                    break;
                case 5:
                    xnode::Erase(node_map_sp, XPath(keys[0], keys[1], keys[2]));
                    break;
                case 6: {
                    auto json    = xnode::ToJson(node_map_sp);
                    auto node_sp = xnode::NodeGet(node_map_sp, XPath(keys[0]), INode::NodeType::Map);
                    ASSERT_TRUE(node_sp);
                    node_sp->ParentDetach();
                    break;
                }
                case 7: {
                    auto json    = xnode::ToJson(node_map_sp);
                    auto node_sp = xnode::NodeGet(node_map_sp, XPath(keys[0], keys[1]), INode::NodeType::Map);
                    ASSERT_TRUE(node_sp);
                    node_sp->ParentDetach();
                    break;
                }
                case 8: {
                    auto json    = xnode::ToJson(node_map_sp);
                    auto node_sp = xnode::NodeGet(node_map_sp, XPath(keys[0], keys[1], keys[2]), INode::NodeType::Map);
                    ASSERT_TRUE(node_sp);
                    node_sp->ParentDetach();
                    break;
                }
                case 9: {
                    xnode::Set(node_map_sp, XPath(keys[0], keys[1], keys[2], "array", 5, value_name), "array_val");
                    xnode::At(node_map_sp, XPath(keys[0], keys[1], keys[2], "array", 5));
                    break;
                }
               
            }

            rounds++;
        }
    };

    size_t                   threads = 8;
    std::vector<std::thread> vec_threads;
    for (size_t z = 0; z < threads; ++z) {
        vec_threads.emplace_back(pf_thread);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    stop = true;
    for (auto& th : vec_threads)
        th.join();

    EXPECT_EQ(xnode::ParentsCheck(node_map_sp, true).size(), 0);

    std::cout << "MAP" << xnode::ToJson(node_map_sp) << std::endl;

    // std::cout << "ARRAY" << xnode::ToJson(node_array_sp) << std::endl;

    std::cout << "rounds:" << std::to_string(rounds.load()) << std::endl;

    std::cout << "FINSIHED" << std::endl;
    // EXPECT_FALSE(TRUE);
}

// NOLINTEND(*)