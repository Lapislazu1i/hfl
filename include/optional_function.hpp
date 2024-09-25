#pragma once
#include <functional>
#include <optional>


namespace hfl
{

template <typename T, typename Func>
constexpr auto fmap(const std::optional<T>& res, Func&& f) -> std::optional<decltype(f(std::declval<T>()))>
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

template <typename T, typename Ret>
constexpr auto applicative(const std::optional<T>& res, const std::optional<std::function<Ret(T)>>& f)
    -> std::optional<Ret>
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

template <typename T, typename Func>
constexpr auto mbind(const std::optional<T>& res, Func&& f) -> decltype(f(std::declval<T>()))
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


} // namespace hfl
