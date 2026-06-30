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
struct is_rs_result : std::false_type
{
};

template<typename T, typename E>
struct is_rs_result<rs_result<T, E>> : std::true_type
{
};

template<typename T>
constexpr bool is_rs_result_v = is_rs_result<T>::value;


template<typename T, typename E>
class rs_result
{
public:
    using ok_type = ok<T>;
    using err_type = err<E>;
    using raw_ok_type = std::remove_cvref_t<T>;
    using raw_err_type = std::remove_cvref_t<E>;

    template<typename FT, typename FE, typename Func>
    friend constexpr auto mbind(const rs_result<FT, FE>& res, Func&& f) -> decltype(f(std::declval<FT>()));

    template<typename FT, typename FE, typename Func>
    friend constexpr auto mbind(rs_result<FT, FE>&& res, Func&& f) -> decltype(f(std::declval<FT>()));

    template<typename U = raw_ok_type>
        requires(!std::is_same_v<std::remove_cvref_t<U>, raw_err_type>)
    constexpr explicit rs_result(U&& val) noexcept(std::is_nothrow_constructible_v<ok_type, U&&>)
        : m_value(std::in_place_type<ok_type>, std::forward<U>(val))
    {
    }

    constexpr explicit rs_result(std::in_place_type_t<ok_type>, raw_ok_type val) noexcept
        : m_value(std::in_place_type<ok_type>, val)
    {
    }

    constexpr rs_result() noexcept : m_value(ok<T>{})
    {
    }

    constexpr rs_result(const ok_type& val) noexcept : m_value(val)
    {
    }

    constexpr rs_result(ok_type&& val) noexcept : m_value(std::move(val))
    {
    }
    constexpr rs_result(const err_type& val) noexcept : m_value(val)
    {
    }

    constexpr rs_result(err_type&& val) noexcept : m_value(std::move(val))
    {
    }

    constexpr bool is_ok() const noexcept
    {
        return std::holds_alternative<ok_type>(m_value);
    }

    constexpr bool is_err() const noexcept
    {
        return std::holds_alternative<err_type>(m_value);
    }

    constexpr void set_ok(ok_type&& v) noexcept
    {
        m_value = std::move(v);
    }

    constexpr void set_ok(const ok_type& v) noexcept
    {
        m_value = v;
    }

    constexpr void set_err(err_type&& v) noexcept
    {
        m_value = std::move(v);
    }

    constexpr void set_err(const err_type& v) noexcept
    {
        m_value = v;
    }

    constexpr std::remove_cvref_t<T> unwrap() const
    {
        if (is_err()) [[unlikely]]
        {
            throw unwrap_exception{inter_unwrap_err()};
        }

        return std::get<ok_type>(m_value).m_value;
    }

    constexpr T unwrap()
    {
        if (is_err()) [[unlikely]]
        {
            throw unwrap_exception{inter_unwrap_err()};
        }

        return std::get<ok_type>(m_value).m_value;
    }

    template<typename U>
        requires std::is_constructible_v<T, U>
    constexpr T unwrap_or_default(U&& val) const noexcept
    {
        if (is_err())
        {
            return val;
        }

        return std::get<ok_type>(m_value).m_value;
    }

    constexpr E unwrap_err() const
    {
        if (is_ok()) [[unlikely]]
        {
            throw unwrap_err_exception{inter_unwrap_ok()};
        }
        return std::get<err_type>(m_value).m_value;
    }

    template<typename OkFunc, typename ErrFunc>
        requires std::is_invocable_v<OkFunc, ok_type&> and std::is_invocable_v<ErrFunc, err_type&>
    constexpr decltype(auto) match(OkFunc&& val_func,
                                   ErrFunc&& err_func) & noexcept(std::is_nothrow_invocable_v<OkFunc, ok_type&> and
                                                                  std::is_nothrow_invocable_v<ErrFunc, err_type&>)
    {
        if (is_ok()) [[likely]]
        {
            return std::invoke(std::forward<OkFunc>(val_func), std::get<ok_type>(m_value));
        }
        else
        {
            return std::invoke(std::forward<ErrFunc>(err_func), std::get<err_type>(m_value));
        }
    }

    template<typename OkFunc, typename ErrFunc>
        requires std::is_invocable_v<OkFunc, const ok_type&> and std::is_invocable_v<ErrFunc, const err_type&>
    constexpr decltype(auto) match(OkFunc&& val_func, ErrFunc&& err_func) const& noexcept(
        std::is_nothrow_invocable_v<OkFunc, const ok_type&> and std::is_nothrow_invocable_v<ErrFunc, const err_type&>)
    {
        if (is_ok()) [[likely]]
        {
            return val_func(std::get<ok_type>(m_value));
        }
        else
        {
            return err_func(std::get<err_type>(m_value));
        }
    }

    template<typename OkFunc, typename ErrFunc>
        requires std::is_invocable_v<OkFunc, ok_type&&> and std::is_invocable_v<ErrFunc, err_type&&>
    constexpr decltype(auto) match(OkFunc&& val_func,
                                   ErrFunc&& err_func) && noexcept(std::is_nothrow_invocable_v<OkFunc, ok_type&&> and
                                                                   std::is_nothrow_invocable_v<ErrFunc, err_type&&>)
    {
        if (is_ok()) [[likely]]
        {
            return val_func(std::move(std::get<ok_type>(m_value)));
        }
        else
        {
            return err_func(std::move(std::get<err_type>(m_value)));
        }
    }

    template<typename OkFunc, typename ErrFunc>
        requires std::is_invocable_v<OkFunc, const ok_type&&> and std::is_invocable_v<ErrFunc, const err_type&&>
    constexpr decltype(auto) match(OkFunc&& val_func, ErrFunc&& err_func) const&& noexcept(
        std::is_nothrow_invocable_v<OkFunc, const ok_type&&> and std::is_nothrow_invocable_v<ErrFunc, const err_type&&>)
    {
        if (is_ok()) [[likely]]
        {
            return val_func(std::move(std::get<ok_type>(m_value)));
        }
        else
        {
            return err_func(std::move(std::get<err_type>(m_value)));
        }
    }

    void swap(rs_result& b) noexcept(std::is_nothrow_swappable_v<std::variant<ok_type, err_type>>)
    {
        std::swap(m_value, b.m_value);
    }

    friend void swap(rs_result& a, rs_result& b) noexcept(std::is_nothrow_swappable_v<std::variant<ok_type, err_type>>)
    {
        a.swap(b);
    }

    template<typename Func>
        requires std::is_invocable_r_v<bool, Func, raw_ok_type>
    constexpr bool is_ok_and(Func&& f) const noexcept(std::is_nothrow_invocable_r_v<bool, Func, raw_ok_type>)
    {
        return match(
            [&f](const ok_type& v) noexcept(std::is_nothrow_invocable_r_v<bool, Func, raw_ok_type>) {
                return std::invoke(std::forward<Func>(f), v.m_value);
            },
            [](const err_type& e) noexcept { return false; });
    }

    template<typename Func>
        requires std::is_invocable_r_v<bool, Func, raw_err_type>
    constexpr bool is_err_and(Func&& f) const noexcept(std::is_nothrow_invocable_r_v<bool, Func, raw_err_type>)
    {
        return match([](const ok_type& v) noexcept { return false; },
                     [&f](const err_type& e) noexcept(std::is_nothrow_invocable_r_v<bool, Func, raw_err_type>) {
                         return std::invoke(std::forward<Func>(f), e.m_value);
                     });
    }


    template<typename Func>
        requires std::is_invocable_v<Func, raw_ok_type>
    auto map(Func&& f) const noexcept(std::is_nothrow_invocable_v<Func, raw_ok_type>)
        -> rs_result<std::invoke_result_t<Func, T>, E>
    {
        using return_type = std::invoke_result_t<Func, T>;
        return match(
            [&f](const ok_type& v) noexcept(std::is_nothrow_invocable_v<Func, raw_ok_type>) {
                return rs_result<return_type, E>{ok<return_type>{f(v.m_value)}};
            },
            [](const err_type& e) noexcept { return rs_result<return_type, E>{e}; });
    }

    template<typename U, typename Func>
        requires std::is_invocable_r_v<U, Func, raw_ok_type>
    auto map_or(U val, Func&& f) const noexcept(std::is_nothrow_invocable_r_v<U, Func, raw_ok_type>) -> U
    {

        return match(
            [&f, &val](const ok_type& v) noexcept(std::is_nothrow_invocable_r_v<U, Func, raw_ok_type>) {
                return std::invoke(std::forward<Func>(f), v.m_value);
            },
            [&val](const err_type&) noexcept { return val; });
    }

    template<typename UFunc, typename Func>
        requires std::is_invocable_r_v<std::invoke_result_t<UFunc>, UFunc>
    auto map_or_else(UFunc&& uf, Func&& f) const
        noexcept(std::is_nothrow_invocable_r_v<raw_ok_type, Func, raw_ok_type> and
                 std::is_nothrow_invocable_r_v<raw_ok_type, UFunc>) -> std::invoke_result_t<UFunc>
    {
        using return_type = std::invoke_result_t<UFunc>;

        return match(
            [&f](const ok_type& v) noexcept(std::is_nothrow_invocable_r_v<raw_ok_type, Func, raw_ok_type>) {
                return std::invoke(std::forward<Func>(f), v.m_value);
            },
            [&uf](const err_type& e) noexcept(std::is_nothrow_invocable_r_v<raw_ok_type, UFunc>) {
                return std::invoke(std::forward<UFunc>(uf));
            });
    }

    template<typename EFunc>
        requires std::is_invocable_v<EFunc, raw_ok_type>
    auto map_err(EFunc&& f) const
        noexcept(std::is_nothrow_constructible_v<rs_result<T, std::invoke_result_t<EFunc, E>>, ok_type> and
                 std::is_nothrow_constructible_v<rs_result<T, std::invoke_result_t<EFunc, E>>, err_type>)
            -> rs_result<T, std::invoke_result_t<EFunc, E>>
    {
        using return_type = std::invoke_result_t<EFunc, E>;
        return match(
            [](const ok_type& v) noexcept(
                std::is_nothrow_constructible_v<rs_result<T, std::invoke_result_t<EFunc, E>>, ok_type>) {
                return rs_result<T, return_type>{v};
            },
            [&f](const err_type& e) noexcept(
                std::is_nothrow_constructible_v<rs_result<T, std::invoke_result_t<EFunc, E>>, err_type>) {
                return rs_result<T, return_type>{err<return_type>{f(e.m_value)}};
            });
    }

    template<typename Func>
        requires std::is_invocable_v<Func, raw_ok_type> and is_rs_result_v<std::invoke_result_t<Func, T>>
    auto and_then(Func&& f) const noexcept(std::is_nothrow_invocable_v<Func, raw_ok_type> and
                                           std::is_nothrow_constructible_v<std::invoke_result_t<Func, T>, err_type>)
        -> std::invoke_result_t<Func, T>
    {
        using return_type = std::invoke_result_t<Func, T>;
        return match(
            [&f](const ok_type& v) noexcept(std::is_nothrow_invocable_v<Func, raw_ok_type>) { return f(v.m_value); },
            [](const err_type& e) noexcept(std::is_nothrow_constructible_v<std::invoke_result_t<Func, T>, err_type>) {
                return return_type{e};
            });
    }

    template<typename Func>
        requires std::is_invocable_v<Func, raw_err_type> and is_rs_result_v<std::invoke_result_t<Func, E>>
    auto or_else(Func&& f) const noexcept -> std::invoke_result_t<Func, E>
    {
        return match([](const ok_type& v) { return std::invoke_result_t<Func, E>{v}; },
                     [&f](const err_type& e) { return f(e.m_value); });
    }

    constexpr auto as_mut() noexcept -> rs_result<raw_ok_type&, raw_err_type&>
    {
        return match(
            [](ok_type& v) noexcept {
                return rs_result<raw_ok_type&, raw_err_type&>{hfl::ok<raw_ok_type&>{v.m_value}};
            },
            [](err_type& e) noexcept {
                return rs_result<raw_ok_type&, raw_err_type&>{hfl::err<raw_err_type&>{e.m_value}};
            });
    }

    constexpr auto as_ref() const noexcept -> rs_result<const raw_ok_type&, const raw_err_type&>
    {
        return match(
            [](const ok_type& v) noexcept {
                return rs_result<const raw_ok_type&, const raw_err_type&>{hfl::ok<const raw_ok_type&>{v.m_value}};
            },
            [](const err_type& e) noexcept {
                return rs_result<const raw_ok_type&, const raw_err_type&>{hfl::err<const raw_err_type&>{e.m_value}};
            });
    }

    constexpr auto copied() const noexcept -> rs_result<raw_ok_type, raw_err_type>
    {
        if constexpr (std::is_copy_constructible_v<raw_ok_type>)
        {

            return match(
                [](const ok_type& v) noexcept {
                    return rs_result<raw_ok_type, raw_err_type>{hfl::ok<raw_ok_type>{v.m_value}};
                },
                [](const err_type& e) noexcept {
                    return rs_result<raw_ok_type, raw_err_type>{hfl::err<raw_err_type>{e.m_value}};
                });
        }
        else
        {
            return rs_result<raw_ok_type, raw_err_type>{hfl::err<raw_err_type>{}};
        }
    }

    constexpr auto expect(const std::string& msg) const noexcept -> raw_ok_type
    {
        if (is_ok()) [[likely]]
        {
            return inter_unwrap_ok().m_value;
        }
        else
        {
            throw unwrap_exception{err_type{}};
        }
    }

private:
    constexpr err_type inter_unwrap_err() const noexcept
    {
        return std::get<err_type>(m_value);
    }

    constexpr ok_type inter_unwrap_ok() const noexcept
    {
        return std::get<ok_type>(m_value);
    }

    std::variant<ok_type, err_type> m_value;
};


template<typename FT, typename FE, typename Func>
    requires std::is_invocable_v<Func, FT> and is_rs_result_v<std::invoke_result_t<Func, FT>>
constexpr auto mbind(const rs_result<FT, FE>& res, Func&& f) noexcept(
    std::is_nothrow_invocable_v<Func, FT> and
    std::is_nothrow_constructible_v<typename std::invoke_result_t<Func, FT>, typename rs_result<FT, FE>::err_type>)
    -> std::invoke_result_t<Func, FT>
{

    return res.match([&f](const rs_result<FT, FE>::ok_type& v) noexcept(
                         std::is_nothrow_invocable_v<Func, FT>) { return f(v.m_value); },
                     [](const rs_result<FT, FE>::err_type& e) noexcept { return std::invoke_result_t<Func, FT>{e}; });
}

template<typename FT, typename FE, typename Func>
    requires std::is_invocable_v<Func, FT&&> and is_rs_result_v<std::invoke_result_t<Func, FT&&>>
constexpr auto mbind(rs_result<FT, FE>&& res, Func&& f) noexcept(
    std::is_nothrow_invocable_v<Func, FT&&> and
    std::is_nothrow_constructible_v<typename std::invoke_result_t<Func, FT&&>, typename rs_result<FT, FE>::err_type>)
    -> std::invoke_result_t<Func, FT&&>
{
    return std::move(res).match(
        [&f](rs_result<FT, FE>::ok_type&& v) noexcept(std::is_nothrow_invocable_v<Func, FT&&>) {
            return f(std::move(v.m_value));
        },
        [](rs_result<FT, FE>::err_type&& e) noexcept { return std::invoke_result_t<Func, FT&&>{std::move(e)}; });
}

template<typename T, typename E, typename Func>
    requires std::is_invocable_v<Func, T> and is_rs_result_v<std::invoke_result_t<Func, T>>
constexpr decltype(auto) operator|(const rs_result<T, E>& res, Func&& f) noexcept(
    std::is_nothrow_invocable_v<Func, T> and
    std::is_nothrow_constructible_v<typename std::invoke_result_t<Func, T>, typename rs_result<T, E>::err_type>)
{
    return mbind<T, E, Func>(res, std::forward<Func>(f));
}

template<typename T, typename E, typename Func>
constexpr decltype(auto) operator|(rs_result<T, E>&& res, Func&& f) noexcept(
    std::is_nothrow_invocable_v<Func, T&&> and
    std::is_nothrow_constructible_v<typename std::invoke_result_t<Func, T&&>, typename rs_result<T, E>::err_type>)
{
    return mbind<T, E, Func>(std::move(res), std::forward<Func>(f));
}

template<typename T, typename E, typename... Funcs>
constexpr decltype(auto) pipeline(const rs_result<T, E>& res, Funcs&&... f)
{
    return ((res | ... | std::forward<Funcs>(f)));
}

template<typename T, typename E, typename... Funcs>
constexpr decltype(auto) pipeline(rs_result<T, E>&& res, Funcs&&... f)
{
    return ((std::move(res) | ... | std::forward<Funcs>(f)));
}

template<typename T, typename E, typename OkFunc, typename ErrFunc>
    requires std::is_invocable_v<OkFunc, typename rs_result<T, E>::ok_type> and
             std::is_invocable_v<ErrFunc, typename rs_result<T, E>::err_type>
constexpr decltype(auto) match(const rs_result<T, E>& res, OkFunc&& val_func, ErrFunc&& err_func) noexcept(
    std::is_nothrow_invocable_v<OkFunc, typename rs_result<T, E>::ok_type> and
    std::is_nothrow_invocable_v<ErrFunc, typename rs_result<T, E>::err_type>)
{
    return res.match(std::forward<OkFunc>(val_func), std::forward<ErrFunc>(err_func));
}

} // namespace hfl