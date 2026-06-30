#pragma once
#include <exception>
#include <functional>
#include <string>
#include <type_traits>
#include <variant>
namespace hfl
{

template<typename T>
struct ok
{
    T m_value;
};

template<typename T>
ok(T) -> ok<T>;

template<typename E>
struct err
{
    E m_value;
};

template<typename E>
err(E) -> err<E>;


template<typename T>
struct some
{
    T m_value;
};

template<typename T>
some(T) -> some<T>;

struct none
{
};

template<typename E>
struct unwrap_exception : public std::exception
{
    unwrap_exception(const err<E>& e) : m_e(e)
    {
    }
    const char* what() const noexcept override
    {
        return "unwrap_exception";
    }
    err<E> m_e;
};


struct unwrap_none_exception : public std::exception
{
    unwrap_none_exception()
    {
    }

    const char* what() const noexcept override
    {
        return "unwrap_none_exception";
    }
};


template<typename T>
struct unwrap_err_exception : public std::exception
{
    unwrap_err_exception(const ok<T>& v) : m_v(v)
    {
    }

    const char* what() const noexcept override
    {
        return "unwrap_err_exception";
    }

    ok<T> m_v;
};

template<typename T, typename E>
class rs_result;

template<typename T>
class rs_option;

}