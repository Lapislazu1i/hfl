#pragma once
#include "optional.hpp"
#include <functional>

namespace hfl
{

template<typename T, typename Func>
constexpr auto fmap(const optional<T>& res, Func&& f) -> optional<decltype(f(std::declval<T>()))>
{
    if (res.has_value())
    {
        return f(res.value());
    }
    else
    {
        return std::nullopt;
    }
}

template<typename T, typename Ret>
constexpr auto applicative(const optional<T>& res, const optional<std::function<Ret(T)>>& f) -> optional<Ret>
{
    if (res.has_value() && f.has_value())
    {
        return f.value()(res.value());
    }
    else
    {
        return std::nullopt;
    }
}

template<typename T, typename Func>
constexpr auto mbind(const optional<T>& res, Func&& f) -> decltype(f(std::declval<T>()))
{
    if (res.has_value())
    {
        return f(res.value());
    }
    else
    {
        return decltype(f(std::declval<T>()))(std::nullopt);
    }
}

template<typename T, typename Func>
constexpr auto operator^(optional<T> res, Func&& f)
{
    return mbind<T, Func>(res, std::forward<Func>(f));
}

template<typename T, typename... Funcs>
constexpr auto pipeline(const optional<T>& res, Funcs&&... f)
{
    return ((res ^ ... ^ f));
}

} // namespace hfl
