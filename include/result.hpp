#pragma once
#include <type_traits>
#include <variant>
#include <system_error>

namespace hfl
{


template<typename T>
class result
{
public:

result() = default;

constexpr result(const T& val) : m_value(val) {}

constexpr result(T&& val) : m_value(std::move(val)) {}

constexpr result(const std::error_code& val) : m_value(val) {}

constexpr result(std::error_code&& val) : m_value(std::move(val)) {}

constexpr bool has_value() const
{
    return std::holds_alternative<T>(m_value);
}

constexpr const T& value() const
{
    return std::get<T>(m_value);
}

constexpr void set_value(T&& v)
{
    m_value = std::move(v);
}

constexpr void set_value(const T& v)
{
    m_value = v;
}

constexpr T unwrap() const
{
    return std::get<T>(m_value);
}

constexpr const std::error_code& error_code() const
{
    return std::get<std::error_code>(m_value);
}

template<typename ValFunc, typename ErrFunc>
constexpr void match(ValFunc&& val_func, ErrFunc&& err_func) const
{
    if(has_value())
    {
        val_func(value());
    }
    else 
    {
        err_func(error_code());
    }
}

friend void swap(result& a, result& b)
{
    std::swap(a.m_value, b.m_value);
}

private:
std::variant<T, std::error_code> m_value;
};


}