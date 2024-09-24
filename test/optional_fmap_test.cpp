#include "optional_function.hpp"
#include <gtest/gtest.h>
#include <optional>


static std::optional<int> get_one()
{
    return 1;
}

static int times_by_two(int val)
{
    return 2 * val;
}

TEST(optional_fmap_test, fmap_invoke_by_lambda)
{
    auto one = get_one();
    auto two_res = hfl::fmap(one, [](int v) { return 2 * v; });
    EXPECT_EQ(2, two_res.value());
}

TEST(optional_fmap_test, fmap_invoke_by_function)
{
    auto one = get_one();
    auto two_res = hfl::fmap(one, &times_by_two);
    EXPECT_EQ(2, two_res.value());
}

TEST(optional_fmap_test, fmap_invoke_with_null_in_val)
{
    auto one = get_one();
    one = (std::nullopt);
    auto two_res = hfl::fmap(one, &times_by_two);
    EXPECT_EQ(false, two_res.has_value());
}

TEST(optional_fmap_test, fmap_invoke_times_by_two)
{
    auto one = get_one();
    one = (std::nullopt);
    auto two_res = hfl::fmap(one, &times_by_two);
    EXPECT_EQ(false, two_res.has_value());
}

TEST(optional_fmap_test, fmap_invoke_times_by_two_times_by_two)
{
    auto one = get_one();
    auto two_res = hfl::fmap(hfl::fmap(one, &times_by_two), &times_by_two);
    EXPECT_EQ(4, two_res.value());
}