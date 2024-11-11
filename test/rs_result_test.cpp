#include "rs_result.hpp"
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

static hfl::rs_result<int, std::string> times_by_two(int val)
{
    return hfl::rs_result<int, std::string>::ok_type{val * 2};
}

static hfl::rs_result<int, std::string> times_by_three(int val)
{
    return hfl::rs_result<int, std::string>::ok_type{val * 3};
}

static hfl::rs_result<int, std::string> append_a(const std::string& val)
{
    std::string tmp = val;
    tmp += "a";
    return hfl::rs_result<int, std::string>::err_type{tmp};
}

static hfl::rs_result<int, std::string> append_b(std::string val)
{
    std::string tmp = val;
    tmp += "b";
    return hfl::rs_result<int, std::string>::err_type{tmp};
}

TEST(rs_result_test, rs_result_match)
{
    int flag{0};
    hfl::rs_result<int, std::string> res{};
    hfl::match(res, [&flag](const hfl::rs_result<int, std::string>::ok_type&){ flag = 1;}, [&flag](const hfl::rs_result<int, std::string>::err_type&){ flag = 2;});
    EXPECT_EQ(1, flag);
}


TEST(rs_result_test, rs_result_match_error)
{
    int flag{0};
    hfl::rs_result<int, std::string> res{};
    res = hfl::rs_result<int, std::string>::err_type{"error"};
    res.match([&flag](hfl::rs_result<int, std::string>::ok_type){ flag = 1;}, [&flag](hfl::rs_result<int, std::string>::err_type){ flag = 2;});
    EXPECT_EQ(2, flag);
}



TEST(rs_result_test, rs_result_swap)
{
    hfl::rs_result<int, std::string> a{hfl::rs_result<int, std::string>::ok_type{1}};
    hfl::rs_result<int, std::string> b{hfl::rs_result<int, std::string>::ok_type{1}};
    std::swap(a, b);
    EXPECT_EQ(1, b.unwrap());
}

TEST(rs_result_test, rs_result_swap_error)
{
    hfl::rs_result<int, std::string> a{hfl::rs_result<int, std::string>::err_type{"error"}};
    hfl::rs_result<int, std::string> b{hfl::rs_result<int, std::string>::ok_type{1}};
    std::swap(a, b);
    EXPECT_EQ(false, b.is_ok());
}

TEST(rs_result_test, rs_result_bind)
{
    auto fun1 = [](double val){return hfl::rs_result<int, std::string>(hfl::ok<int>{34});};
    hfl::rs_result<int, std::string> a{hfl::rs_result<int, std::string>::ok_type{2}};
    auto res =hfl::mbind( hfl::mbind(a,[](auto val){
        return hfl::rs_result<double, std::string>{hfl::rs_result<double, std::string>::ok_type{2.5}};
    }),(std::move(fun1)));
    EXPECT_EQ(34, res.unwrap());
}

TEST(rs_result_test, rs_result_pipeline)
{
    hfl::rs_result<int, std::string> a{hfl::ok{1}};

    auto res = hfl::pipeline(a, times_by_two, times_by_three);

    EXPECT_EQ(6, res.unwrap());
}


TEST(rs_result_test, rs_result_pipeline_operator)
{
    hfl::rs_result<int, std::string> a{hfl::ok{1}};

    auto res = a | times_by_two | times_by_three;

    EXPECT_EQ(6, res.unwrap());
}

TEST(rs_result_test, rs_result_is_ok_and)
{
    hfl::rs_result<int, float> a{hfl::ok{1}};
    auto b1 = a.is_ok_and([](int v){return v > 0;});
    auto b2 = a.is_ok_and([](int v){return v < 0;});
    a = hfl::err(4.f);
    auto b3 = a.is_ok_and([](int v){return v > 0;});

    EXPECT_EQ(true, b1);
    EXPECT_EQ(false, b2);
    EXPECT_EQ(false, b3);
}

TEST(rs_result_test, rs_result_is_err_and)
{
    hfl::rs_result<int, float> a{hfl::err{1.0f}};
    auto b1 = a.is_err_and([](float v){return v > 0;});
    auto b2 = a.is_err_and([](float v){return v < 0;});
    a = hfl::ok(4);
    auto b3 = a.is_err_and([](float v){return v > 0;});

    EXPECT_EQ(true, b1);
    EXPECT_EQ(false, b2);
    EXPECT_EQ(false, b3);
}

TEST(rs_result_test, rs_result_map)
{
    hfl::rs_result<int, float> a{hfl::ok<int>{3}};
    auto b = a.map([](int v) -> double
    {
        return double(2 * v);
    });

    hfl::rs_result<int, float> c{hfl::err<float>{3.f}};
    auto d = c.map([](int v) -> double
    {
        return double(2 * v);
    });

    EXPECT_EQ(6, b.unwrap());
    EXPECT_EQ(true, d.is_err());
}

TEST(rs_result_test, rs_result_map_or)
{
    hfl::rs_result<int, float> a{hfl::ok<int>{3}};
    auto b = a.map_or(22.3, [](hfl::ok<int> v) -> hfl::rs_result<double, float>
    {
        return hfl::ok<double>(2 * v.m_value);
    });

    hfl::rs_result<int, float> c{hfl::err<float>{3.f}};
    auto d = c.map_or(22.3, [](hfl::ok<int> v) -> hfl::rs_result<double, float>
    {
        return hfl::ok<double>(2 * v.m_value);
    });

    EXPECT_EQ(6.0, b);
    EXPECT_EQ(22.3, d);
}

TEST(rs_result_test, rs_result_map_or_else)
{
    hfl::rs_result<int, float> a{hfl::ok<int>{3}};
    auto b = a.map_or_else([](hfl::err<float>){return 22.3;}, [](hfl::ok<int> v) -> hfl::rs_result<double, float>
    {
        return hfl::ok<double>(2 * v.m_value);
    });

    hfl::rs_result<int, float> c{hfl::err<float>{3.f}};
    auto d = c.map_or_else([](hfl::err<float>){return 22.3;}, [](hfl::ok<int> v) -> hfl::rs_result<double, float>
    {
        return hfl::ok<double>(2 * v.m_value);
    });

    EXPECT_EQ(6.0, b);
    EXPECT_EQ(22.3, d);
}

TEST(rs_result_test, rs_result_map_err)
{
    hfl::rs_result<int, float> a{hfl::err<float>{3}};
    auto b = a.map_err([](float v)
    {
        return int(2 * v);
    });

    hfl::rs_result<int, float> c{hfl::ok<int>{3}};
    auto d = c.map_err([](float v)
    {
        return int(2 * v);
    });

    EXPECT_EQ(6, b.unwrap_err());
    EXPECT_EQ(true, b.is_err());
    EXPECT_EQ(true, d.is_ok());
}

TEST(rs_result_test, rs_result_unwrap_or_default)
{
    hfl::rs_result<int, float> a{hfl::err<float>{3}};
    auto r1 = a.unwrap_or_default(23);
    hfl::rs_result<int, float> b{hfl::ok{3}};
    auto r2 = b.unwrap_or_default(23);
    EXPECT_EQ(23, r1);
    EXPECT_EQ(3, r2);
}

TEST(rs_result_test, rs_result_and_then)
{
    hfl::rs_result<int, std::string> b{hfl::ok{3}};
    hfl::rs_result<int, std::string> c{hfl::err<std::string>{"hh"}};
    auto r1 = b.and_then(times_by_two).and_then(times_by_three);
    auto r2 = c.and_then(times_by_two).and_then(times_by_three);
    
    EXPECT_EQ(3 * 2 * 3, r1.unwrap());
    EXPECT_EQ(true, r2.is_err());
    EXPECT_EQ(false, r2.is_ok());
}

TEST(rs_result_test, rs_result_or_else)
{
    hfl::rs_result<int, std::string> b{hfl::ok{3}};
    hfl::rs_result<int, std::string> c{hfl::err<std::string>{"c"}};
    auto r1 = b.or_else(append_a).or_else(append_b);
    auto r2 = c.or_else(append_a).or_else(append_b);
    
    EXPECT_EQ(true, r1.is_ok());
    EXPECT_EQ(true, r2.is_err());
    EXPECT_EQ("cab", r2.unwrap_err());
}

TEST(rs_result_test, rs_result_as_mut)
{
    hfl::rs_result<int, std::string> b{hfl::ok{3}};
    // hfl::rs_result<int, std::string> c{hfl::err<std::string>{"c"}};

    auto rr1 = b.unwrap();

    auto r1 = b.as_mut();

    auto rr2 = r1.unwrap();
    
    auto& t1 = r1.unwrap();
    t1 = 22;
    EXPECT_EQ(true, r1.is_ok());
    EXPECT_EQ(22, r1.unwrap());

}

TEST(rs_result_test, rs_result_as_ref)
{
    hfl::rs_result<int, std::string> b{hfl::ok{3}};
    hfl::rs_result<int, std::string> c{hfl::err<std::string>{"c"}};
    
    auto r1 = b.as_ref();
    const int& t1 = r1.unwrap();
    EXPECT_EQ(true, r1.is_ok());
}

TEST(rs_result_test, rs_result_set_ok)
{
    hfl::rs_result<int, std::string> b{};
    hfl::ok<int> r1{1};
    b.set_ok(r1);
    EXPECT_EQ(1, b.unwrap());
    r1.m_value = 2;
    b.set_ok(std::move(r1));
    EXPECT_EQ(2, b.unwrap());
}

TEST(rs_result_test, rs_result_set_err)
{
    hfl::rs_result<int, std::string> b{};
    hfl::err<std::string> r1{"1"};
    b.set_err(r1);
    EXPECT_EQ("1", b.unwrap_err());
    r1.m_value = "2";
    b.set_err(std::move(r1));
    EXPECT_EQ("2", b.unwrap_err());
}

TEST(rs_result_test, rs_result_coped)
{
    hfl::rs_result<int, float> b{hfl::ok{2}};
    hfl::rs_result<int&, float&> r1 = b.as_mut();
    hfl::rs_result<int, float> r2 = b.copied();

    EXPECT_EQ(2, b.unwrap());
}

TEST(rs_result_test, rs_result_test_move_mbind)
{
    hfl::rs_result<test_move_string, float> b{hfl::ok{test_move_string{"123"}}};\
    hfl::rs_result<test_move_string, float> c = b;
    auto f1 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "a";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto f2 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "b";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto r1 = hfl::mbind(std::move(b), f1);
    EXPECT_EQ(true, b.unwrap().m_buf.empty());
    EXPECT_EQ("123a", r1.unwrap().m_buf);

    auto r2 = hfl::mbind(std::move(r1), f2);
    EXPECT_EQ(true, r1.unwrap().m_buf.empty());
    EXPECT_EQ("123ab", r2.unwrap().m_buf);
    auto r3 = hfl::mbind(hfl::mbind(std::move(c) , f2), f1);
    EXPECT_EQ("123ba", r3.unwrap().m_buf);
    EXPECT_EQ(true, c.unwrap().m_buf.empty());

}

TEST(rs_result_test, rs_result_test_move_pipe_operator)
{
    hfl::rs_result<test_move_string, float> b{hfl::ok{test_move_string{"123"}}};\
    hfl::rs_result<test_move_string, float> c = b;
    auto f1 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "a";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto f2 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "b";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto r1 = std::move(b) | f1;
    EXPECT_EQ(true, b.unwrap().m_buf.empty());
    EXPECT_EQ("123a", r1.unwrap().m_buf);

    auto r2 = std::move(r1) | f2;
    EXPECT_EQ(true, r1.unwrap().m_buf.empty());
    EXPECT_EQ("123ab", r2.unwrap().m_buf);
    auto r3 = std::move(c) | f2 | f1;
    EXPECT_EQ("123ba", r3.unwrap().m_buf);
    EXPECT_EQ(true, c.unwrap().m_buf.empty());
}

TEST(rs_result_test, rs_result_test_move_pipeline)
{
    hfl::rs_result<test_move_string, float> b{hfl::ok{test_move_string{"123"}}};\
    hfl::rs_result<test_move_string, float> c = b;
    auto f1 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "a";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto f2 = [](test_move_string&& val){
        test_move_string tmp1 = std::move(val);
        tmp1.m_buf += "b";
        return hfl::rs_result<test_move_string, float>(hfl::ok<test_move_string>{std::move(tmp1)});
    };
    auto r1 = hfl::pipeline(std::move(b) , f1);
    EXPECT_EQ(true, b.unwrap().m_buf.empty());
    EXPECT_EQ("123a", r1.unwrap().m_buf);

    auto r2 = hfl::pipeline(std::move(r1) , f2);
    EXPECT_EQ(true, r1.unwrap().m_buf.empty());
    EXPECT_EQ("123ab", r2.unwrap().m_buf);
    auto r3 = hfl::pipeline(std::move(c) , f2 , f1);
    EXPECT_EQ("123ba", r3.unwrap().m_buf);
    EXPECT_EQ(true, c.unwrap().m_buf.empty());
}