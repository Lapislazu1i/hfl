#include "memo.hpp"
#include "timer.hpp"
#include <gtest/gtest.h>

constexpr std::int64_t sleep_time = 800;

static int times_by_2(int v)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    return 2 * v;
}

TEST(memo_test, normal_invoke)
{
    auto mem_func = hfl::make_memo<int(int)>(&times_by_2);
    hfl::timer timer{};
    auto res1 = mem_func(2);
    timer.end();
    auto duration1 = timer.elapsed_time();

    timer.start();
    auto res2 = mem_func(2);
    timer.end();
    auto duration2 = timer.elapsed_time();

    auto in_d1 = hfl::in_duration(duration1.count(), sleep_time);
    auto in_d2 = hfl::in_duration(duration2.count(), 1, 10);

    EXPECT_EQ(4, res1);
    EXPECT_EQ(4, res2);
    EXPECT_EQ(true, in_d2);
    EXPECT_EQ(true, in_d2);
}

TEST(memo_test, normal_invoke_by_lambda)
{
    auto mem_func = hfl::make_memo<int(int)>([](int v) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        return 2 * v;
    });
    hfl::timer timer{};
    auto res1 = mem_func(2);
    timer.end();
    auto duration1 = timer.elapsed_time();

    timer.start();
    auto res2 = mem_func(2);
    timer.end();
    auto duration2 = timer.elapsed_time();

    auto in_d1 = hfl::in_duration(duration1.count(), sleep_time);
    auto in_d2 = hfl::in_duration(duration2.count(), 1, 10);

    EXPECT_EQ(4, res1);
    EXPECT_EQ(4, res2);
    EXPECT_EQ(true, in_d2);
    EXPECT_EQ(true, in_d2);
}

TEST(memo_test, recursive_invoke_with_lambda)
{
    auto mem_func = hfl::make_recursive_memo<int(int)>([](auto& f, int v) {
        if (v == 0)
        {
            return 11;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        return f(v - 1);
    });

    hfl::timer timer{};
    auto res1 = mem_func(2);
    timer.end();
    auto duration1 = timer.elapsed_time();

    timer.start();
    auto res2 = mem_func(2);
    timer.end();
    auto duration2 = timer.elapsed_time();

    auto in_d1 = hfl::in_duration(duration1.count(), sleep_time * 2);
    auto in_d2 = hfl::in_duration(duration2.count(), 1, 10);

    EXPECT_EQ(11, res1);
    EXPECT_EQ(11, res2);
    EXPECT_EQ(true, in_d2);
    EXPECT_EQ(true, in_d2);
}

static uint64_t fib_f(uint64_t n)
{
    return n == 0 ? 0 : n == 1 ? 1 : fib_f(n - 1) + fib_f(n - 2);
}

TEST(memo_test, recursive_invoke_fib_with_lambda)
{
    auto fibmemo = hfl::make_recursive_memo<uint64_t(uint64_t)>([](auto& fib, uint64_t n) -> uint64_t {
        return n == 0 ? 0 : n == 1 ? 1 : fib(n - 1) + fib(n - 2);
    });

    auto dd = [](auto& fib, uint64_t n) -> uint64_t { return n == 0 ? 0 : n == 1 ? 1 : fib(n - 1) + fib(n - 2); };

    auto fib_argument = 4;
    auto res = fibmemo(fib_argument);
    auto res2 = fib_f(fib_argument);
    EXPECT_EQ(res, res2);
}