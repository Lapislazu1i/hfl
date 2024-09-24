#include "result_function.hpp"
#include <gtest/gtest.h>
#include <system_error>



TEST(result_applicative_test, applicative_normal_invoke)
{
    hfl::result<int> one{1};
    hfl::result<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1; }};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(2, res.value());
}

TEST(result_applicative_test, applicative_normal_invoke_val_with_null)
{
    hfl::result<int> one{std::make_error_code(std::errc::io_error)};
    hfl::result<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1; }};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}

TEST(result_applicative_test, applicative_normal_invoke_func_with_null)
{
    hfl::result<int> one{1};
    hfl::result<std::function<int(int)>> add_one_fuc{std::make_error_code(std::errc::io_error)};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}

TEST(result_applicative_test, applicative_normal_invoke_val_and_func_both_with_null)
{
    hfl::result<int> one{std::make_error_code(std::errc::io_error)};
    hfl::result<std::function<int(int)>> add_one_fuc{std::make_error_code(std::errc::io_error)};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}


TEST(result_applicative_test, applicative_normal_invoke_add_one_add_one)
{
    hfl::result<int> one{1};
    hfl::result<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1;}};
    auto res = hfl::applicative(hfl::applicative(one, add_one_fuc), add_one_fuc);
    EXPECT_EQ(3, res.value());
}