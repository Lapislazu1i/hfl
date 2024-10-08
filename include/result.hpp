#pragma once
#include <system_error>
#include <type_traits>
#include <variant>

namespace hfl
{


template<typename T>
class result
{
public:
    result() noexcept = default;

    constexpr result(const T& val) noexcept : m_value(val)
    {
    }

    constexpr result(T&& val) : m_value(std::move(val))
    {
    }

    constexpr result(const std::error_code& val) noexcept : m_value(val)
    {
    }

    constexpr result(std::error_code&& val) noexcept : m_value(std::move(val))
    {
    }

    constexpr bool has_value() const noexcept
    {
        return std::holds_alternative<T>(m_value);
    }

    constexpr const T& value() const&
    {
        return std::get<T>(m_value);
    }

    constexpr T value() &&
    {
        return std::get<T>(m_value);
    }

    constexpr T value_or(T&& right) const noexcept
    {
        if (has_value())
        {
            return value();
        }
        return right;
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

    constexpr std::error_code error_code() const
    {
        return std::get<std::error_code>(m_value);
    }

    template<typename ValFunc, typename ErrFunc>
    constexpr void match(ValFunc&& val_func, ErrFunc&& err_func) const
    {
        if (has_value())
        {
            val_func(unwrap());
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

    template<typename Func>
    constexpr auto bind(Func&& f) -> decltype(f(std::declval<T>()))
    {
        if (this->has_value())
        {
            return f(this->unwrap());
        }
        else
        {
            return decltype(f(std::declval<T>())){this->error_code()};
        }
    }


private:
    std::variant<T, std::error_code> m_value;
};


} // namespace hfl