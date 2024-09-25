#include "result.hpp"
#include "result_function.hpp"
#include <gtest/gtest.h>
#include <system_error>


static hfl::result<int> add_one(int val)
{
    return val + 1;
}

static hfl::result<int> times_by_two(int val)
{
    return val * 2;
}

static hfl::result<int> times_by_three(int val)
{
    return val * 3;
}

TEST(result_mbind_test, mbind_normal_invoke)
{
    hfl::result<int> two{2};
    auto res = hfl::mbind(two, &add_one);
    EXPECT_EQ(3, res.value());
}

TEST(result_mbind_test, mbind_normal_invoke_val_with_null)
{
    hfl::result<int> two{std::make_error_code(std::errc::io_error)};
    auto res = hfl::mbind(two, &add_one);
    EXPECT_EQ(false, res.has_value());
}


TEST(result_mbind_test, mbind_normal_invoke_func_with_null)
{
    hfl::result<int> two{2};
    auto res = hfl::mbind(two, [](int v)-> hfl::result<int>{
      return std::make_error_code(std::errc::io_error);
    });
    EXPECT_EQ(false, res.has_value());
}

TEST(result_mbind_test, mbind_normal_invoke_val_func_both_with_null)
{
    hfl::result<int> two{std::make_error_code(std::errc::io_error)};
    auto res = hfl::mbind(two, [](int v)-> hfl::result<int>{
      return std::make_error_code(std::errc::io_error);
    });
    EXPECT_EQ(false, res.has_value());
}

TEST(result_mbind_test, mbind_times_two_times_three)
{
    hfl::result<int> one {1};
    auto res = hfl::mbind(hfl::mbind(one, &times_by_two), &times_by_three);
    EXPECT_EQ(6, res.value());
}

TEST(result_mbind_test, mbind_times_two_times_three_by_pipe)
{
    hfl::result<int> one {1};
    auto res = one | times_by_two | times_by_three;
    EXPECT_EQ(6, res.value());
}

TEST(result_mbind_test, mbind_times_two_times_three_by_pipe_using_lambda)
{
    hfl::result<int> one {1};
    auto res = one | [](int v) -> hfl::result<int>{return v * 2;} | [](int v) -> hfl::result<int> { return v * 3;};
    EXPECT_EQ(6, res.value());
}

TEST(result_mbind_test, mbind_int_to_double)
{
    hfl::result<int> one {1};
    auto res = one | [](int v) -> hfl::result<double>{return (double)v;};
    const auto& dinfo = typeid(double);
    const auto& rinfo = typeid(decltype(res.unwrap()));
    EXPECT_EQ(dinfo, rinfo);
}

