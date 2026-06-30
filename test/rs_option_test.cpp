#include "rs_option.hpp"
#include <gtest/gtest.h>
#include <system_error>
#include <string>

struct test_move_string
{
    test_move_string(const char* str) : m_buf(str) {}
    test_move_string(const test_move_string& v) 
    {
        m_buf = v.m_buf;
    }
    test_move_string(test_move_string&& v) 
    {
        m_buf = v.m_buf;
        v.m_buf = std::string{};
    }
    test_move_string& operator=(const test_move_string& v) 
    {
        m_buf = v.m_buf;
        return *this;
    }
    test_move_string& operator=(test_move_string&& v) 
    {
        m_buf = v.m_buf;
        v.m_buf = std::string{};
        return *this;
    }
    std::string m_buf;
};


TEST(rs_option_test, rs_option_match)
{
    hfl::rs_option<int> r1{1};
}