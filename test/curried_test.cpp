#include "curried.hpp"
#include "timer.hpp"
#include <gtest/gtest.h>

static int return_int(int b)
{
    return b;
}

TEST(curried_test, normal_invoke_int_ret_int)
{
    hfl::curried<int(int)> cur(return_int);
    auto res = cur(2);
    EXPECT_EQ(2, res);
}


TEST(curried_test, lambda_invoke_int_ret_int)
{
    hfl::curried<int(int)> cur([](int a){return a;});
    auto res = cur(2);
    EXPECT_EQ(2, res);
}


TEST(curried_test, lambda_invoke_sum_int_int)
{
    hfl::curried<int(int, int)> cur([](int a, int b){return a + b;});
    auto res = cur(2)(3);
    EXPECT_EQ(5, res);
}