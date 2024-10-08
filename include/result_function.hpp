#pragma once
#include "result.hpp"
#include <functional>

namespace hfl
{

template<typename T, typename Func>
constexpr auto fmap(const result<T>& opt, Func&& f) -> result<decltype(f(std::declval<T>()))>
{
    if (opt.has_value())
    {
        return f(opt.unwrap());
    }
    else
    {
        return result<decltype(f(std::declval<T>()))>(opt.error_code());
    }
}

template<typename T, typename Ret>
constexpr auto applicative(const result<T>& res, const result<std::function<Ret(T)>>& f) -> result<Ret>
{
    if (res.has_value() && f.has_value())
    {
        return f.value()(res.value());
    }
    else
    {
        if (!res.has_value())
        {
            return result<Ret>(res.error_code());
        }

        return result<Ret>(f.error_code());
    }
}

template<typename T, typename Func>
constexpr auto mbind(const result<T>& res, Func&& f) -> decltype(f(std::declval<T>()))
{
    if (res.has_value())
    {
        return f(res.unwrap());
    }
    else
    {
        return decltype(f(std::declval<T>()))(res.error_code());
    }
}

template<typename T, typename Func>
constexpr auto operator^(result<T> res, Func&& f)
{
    return mbind<T, Func>(res, std::forward<Func>(f));
}

template<typename T, typename... Funcs>
constexpr auto pipeline(const result<T>& res, Funcs&&... f)
{
    return ((res ^ ... ^ f));
}

} // namespace hfl
