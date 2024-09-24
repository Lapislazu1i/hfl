#include "optional_function.hpp"
#include <gtest/gtest.h>
#include <optional>


TEST(optional_applicative_test, applicative_normal_invoke)
{
    std::optional<int> one{1};
    std::optional<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1; }};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(2, res.value());
}

TEST(optional_applicative_test, applicative_normal_invoke_val_with_null)
{
    std::optional<int> one{std::nullopt};
    std::optional<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1; }};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}

TEST(optional_applicative_test, applicative_normal_invoke_func_with_null)
{
    std::optional<int> one{1};
    std::optional<std::function<int(int)>> add_one_fuc{std::nullopt};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}

TEST(optional_applicative_test, applicative_normal_invoke_val_and_func_both_with_null)
{
    std::optional<int> one{std::nullopt};
    std::optional<std::function<int(int)>> add_one_fuc{std::nullopt};
    auto res = hfl::applicative(one, add_one_fuc);
    EXPECT_EQ(false, res.has_value());
}

TEST(optional_applicative_test, applicative_normal_invoke_add_one_add_one)
{
    std::optional<int> one{1};
    std::optional<std::function<int(int)>> add_one_fuc{[](int v) { return v + 1;}};
    auto res = hfl::applicative(hfl::applicative(one, add_one_fuc), add_one_fuc);
    EXPECT_EQ(3, res.value());
}