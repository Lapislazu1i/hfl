#include "result.hpp"
#include "result_function.hpp"
#include <gtest/gtest.h>
#include <system_error>

static hfl::result<int> times_by_two(int val)
{
    return val * 2;
}

static hfl::result<int> times_by_three(int val)
{
    return val * 3;
}

TEST(result_test, result_match)
{
    int flag{0};
    hfl::result<int> res{};
    res.match([&flag](int){ flag = 1;}, [&flag](const std::error_code&){ flag = 2;});
    EXPECT_EQ(1, flag);
}

TEST(result_test, result_match_error)
{
    int flag{0};
    hfl::result<int> res{};
    res = std::make_error_code(std::errc::io_error);
    res.match([&flag](int){ flag = 1;}, [&flag](const std::error_code&){ flag = 2;});
    EXPECT_EQ(2, flag);
}

TEST(result_test, result_swap)
{
    hfl::result<int> a{1};
    hfl::result<int> b{2};
    std::swap(a, b);
    EXPECT_EQ(1, b.value());
}

TEST(result_test, result_swap_error)
{
    hfl::result<int> a{std::make_error_code(std::errc::io_error)};
    hfl::result<int> b{2};
    std::swap(a, b);
    EXPECT_EQ(false, b.has_value());
}

TEST(result_test, result_bind)
{
    auto fun1 = [](const double& val){return hfl::result<int>(34);};
    hfl::result<int> a{2};
    auto res = a.bind([](int val){
        return hfl::result<double>(2.5);
    }).bind(fun1);
    EXPECT_EQ(34, res.value());
}

TEST(result_test, result_pipeline)
{
    auto fun1 = [](const double& val){return hfl::result<int>(34);};
    auto fun2 = [](int val){
        return hfl::result<double>(2.5);
    };
    hfl::result<int> a{1};
    auto res = hfl::pipeline(a, times_by_two, times_by_three);
    EXPECT_EQ(6, res.value());
}