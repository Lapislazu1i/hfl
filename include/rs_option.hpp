#pragma once
#include "rs_common.hpp"
#include <exception>
#include <functional>
#include <string>
#include <type_traits>
#include <variant>

namespace hfl
{

template<typename T>
struct is_rs_option : std::false_type
{
};

template<typename T>
struct is_rs_option<rs_option<T>> : std::true_type
{
};

template<typename T>
constexpr bool is_rs_option_v = is_rs_option<T>::value;


template<typename T>
class rs_option
{
public:
    using some_type = some<T>;
    using raw_some_type = T;

    template<typename U>
    constexpr explicit rs_option(U&& val) noexcept(std::is_nothrow_constructible_v<some_type, U&&>)
        : m_value(std::in_place_type<some_type>, std::forward<U>(val))
    {
    }

    constexpr explicit rs_option(const some_type& val) noexcept : m_value(val)
    {
    }

    constexpr explicit rs_option(some_type&& val) noexcept : m_value(std::move(val))
    {
    }

    constexpr explicit rs_option(none val) noexcept : m_value(val)
    {
    }

    constexpr bool is_some() const noexcept
    {
        return std::holds_alternative<some_type>(m_value);
    }

    constexpr bool is_none() const noexcept
    {
        return std::holds_alternative<none>(m_value);
    }

    constexpr std::remove_cvref_t<T> unwrap() const
    {
        if (is_none()) [[unlikely]]
        {
            throw unwrap_none_exception{};
        }

        return std::get<some_type>(m_value).m_value;
    }

    constexpr T unwrap()
    {
        if (is_none()) [[unlikely]]
        {
            throw unwrap_none_exception{};
        }

        return std::get<some_type>(m_value).m_value;
    }

    template<typename U>
        requires std::is_constructible_v<T, U>
    constexpr T unwrap_or_default(U&& val) const noexcept
    {
        if (is_none())
        {
            return val;
        }

        return std::get<some_type>(m_value).m_value;
    }

    template<typename OkFunc, typename ErrFunc>
        requires std::is_invocable_v<OkFunc, some_type&> and std::is_invocable_v<ErrFunc>
    constexpr decltype(auto) match(OkFunc&& val_func,
                                   ErrFunc&& err_func) & noexcept(std::is_nothrow_invocable_v<OkFunc, some_type&> and
                                                                  std::is_nothrow_invocable_v<ErrFunc>)
    {
        if (is_none()) [[likely]]
        {
            return std::invoke(std::forward<OkFunc>(val_func), std::get<some_type>(m_value));
        }
        else
        {
            return std::invoke(std::forward<ErrFunc>(err_func));
        }
    }

private:
    constexpr none inter_unwrap_none() const noexcept
    {
        return std::get<none>(m_value);
    }

    constexpr some_type inter_unwrap_some() const noexcept
    {
        return std::get<some_type>(m_value);
    }
    std::variant<some_type, none> m_value;
};

} // namespace hfl