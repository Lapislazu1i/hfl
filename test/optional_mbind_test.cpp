#include "optional_function.hpp"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <optional>
#include <type_traits>


static std::optional<int> add_one(int val)
{
    return val + 1;
}

static std::optional<int> times_by_two(int val)
{
    return val * 2;
}

static std::optional<int> times_by_three(int val)
{
    return val * 3;
}


TEST(optional_mbind_test, mbind_normal_invoke)
{
    std::optional<int> two{2};
    auto res = hfl::mbind(two, &add_one);
    EXPECT_EQ(3, res.value());
}

TEST(optional_mbind_test, mbind_normal_invoke_val_with_null)
{
    std::optional<int> two{std::nullopt};
    auto res = hfl::mbind(two, &add_one);
    EXPECT_EQ(false, res.has_value());
}


TEST(optional_mbind_test, mbind_normal_invoke_func_with_null)
{
    std::optional<int> two{2};
    auto res = hfl::mbind(two, [](int v)-> std::optional<int>{
      return std::nullopt;
    });
    EXPECT_EQ(false, res.has_value());
}

TEST(optional_mbind_test, mbind_normal_invoke_val_func_both_with_null)
{
    std::optional<int> two{std::nullopt};
    auto res = hfl::mbind(two, [](int v)-> std::optional<int>{
      return std::nullopt;
    });
    EXPECT_EQ(false, res.has_value());
}


TEST(optional_mbind_test, mbind_times_two_times_three)
{
    std::optional<int> one {1};
    auto res = hfl::mbind(hfl::mbind(one, &times_by_two), &times_by_three);
    EXPECT_EQ(6, res.value());
}

TEST(optional_mbind_test, mbind_times_two_times_three_by_pipe)
{
    using namespace hfl;
    std::optional<int> one {1};
    auto res = one ^ &times_by_two ^ &times_by_three;
    EXPECT_EQ(6, res.value());
}

TEST(optional_mbind_test, mbind_int_to_double)
{
    std::optional<int> one {1};
    auto res = hfl::mbind(one,  [](int v) -> std::optional<double>{return (double)v;});
    const auto& dinfo = typeid(double);
    const auto& rinfo = typeid(std::remove_cv_t<decltype(res.value())>);
    EXPECT_EQ(dinfo, rinfo);
}


TEST(optional_mbind_test, mbind_int_to_double_by_pipeline)
{
    std::optional<int> one {1};
    auto fun1 = [](int v) -> std::optional<double>{return (double)v;};
    auto res = hfl::pipeline(one, fun1);
    const auto& dinfo = typeid(double);
    const auto& rinfo = typeid(std::remove_cv_t<decltype(res.value())>);
    EXPECT_EQ(dinfo, rinfo);
}

TEST(optional_mbind_test, mbind_int_to_double_by_pipe)
{
    using namespace hfl;
    hfl::optional<int> one {1};
    auto res = one ^ [](int v) -> hfl::optional<double>{return (double)v;};
    const auto& dinfo = typeid(double);
    const auto& rinfo = typeid(std::remove_cv_t<decltype(res.value())>);
    EXPECT_EQ(dinfo, rinfo);
}

TEST(optional_mbind_test, mbind_by_pipeline)
{
    std::optional<int> one {1};
    // auto res = hfl::mbind(hfl::mbind(one, times_by_two), times_by_two);
    auto res = hfl::pipeline(one, times_by_two, times_by_three);
    
    EXPECT_EQ(6, res.value());
}