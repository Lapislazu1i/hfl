#pragma once
#include <exception>
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

template<typename E>
struct unwrap_exception : public std::exception
{
    unwrap_exception(const err<E>& e) : m_e(e)
    {
    }
    err<E> m_e;
};

template<typename T>
struct unwrap_err_exception : public std::exception
{
    unwrap_err_exception(const ok<T>& v) : m_v(v)
    {
    }
    ok<T> m_v;
};


template<typename T, typename E>
class rs_result
{
public:
    using ok_type = ok<T>;
    using err_type = err<E>;
    using row_ok_type = std::remove_cvref_t<T>;
    using row_err_type = std::remove_cvref_t<E>;

    template<typename FT, typename FE, typename Func>
    friend constexpr auto mbind(const rs_result<FT, FE>& res, Func&& f) -> decltype(f(std::declval<FT>()));

    rs_result() noexcept = default;

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
        if (is_err())
        {
            throw unwrap_exception{inter_unwrap_err()};
        }

        return std::get<ok_type>(m_value).m_value;
    }

    constexpr T unwrap()
    {
        if (is_err())
        {
            throw unwrap_exception{inter_unwrap_err()};
        }

        return std::get<ok_type>(m_value).m_value;
    }

    template<typename U>
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
        if (is_ok())
        {
            throw unwrap_err_exception{inter_unwrap_ok()};
        }
        return std::get<err_type>(m_value).m_value;
    }

    template<typename OkFunc, typename ErrFunc>
    constexpr decltype(auto) match(OkFunc&& val_func, ErrFunc&& err_func) const noexcept
    {
        if (is_ok())
        {
            return val_func(std::get<ok_type>(m_value));
        }
        else
        {
            return err_func(std::get<err_type>(m_value));
        }
    }

    template<typename OkFunc, typename ErrFunc>
    constexpr decltype(auto) match(OkFunc&& val_func, ErrFunc&& err_func) noexcept
    {
        if (is_ok())
        {
            return val_func(std::get<ok_type>(m_value));
        }
        else
        {
            return err_func(std::get<err_type>(m_value));
        }
    }

    void swap(rs_result& b)
    {
        std::swap(m_value, b.m_value);
    }

    friend void swap(rs_result& a, rs_result& b)
    {
        a.swap(b);
    }

    template<typename Func>
    constexpr bool is_ok_and(Func&& f) const noexcept
    {
        return match(
            [&f](const ok_type& v) {
                return f(v.m_value);
            },
            [](const err_type& e) {
                return false;
            });
    }

    template<typename Func>
    constexpr bool is_err_and(Func&& f) const noexcept
    {
        return match(
            [](const ok_type& v) {
                return false;
            },
            [&f](const err_type& e) {
                return f(e.m_value);
            });
    }


    template<typename Func>
    auto map(Func&& f) -> rs_result<decltype(f(std::declval<T>())), E> const
    {
        using return_type = decltype(f(std::declval<T>()));
        return match(
            [&f](const ok_type& v) {
                return rs_result<return_type, E>{ok<return_type>{f(v.m_value)}};
            },
            [](const err_type& e) {
                return rs_result<return_type, E>{e};
            });
    }

    template<typename U, typename Func>
    auto map_or(U val, Func&& f) -> U const
    {
        return match(
            [&f, &val](const ok_type& v) {
                return f(v).match([](const hfl::ok<U>& v){ return v.m_value; }, [&val](const err_type&){return val;});
            },
            [&val](const err_type&) {
                return val;
            });
    }

    template<typename UFunc, typename Func>
    auto map_or_else(UFunc&& uf, Func&& f) -> decltype(uf(std::declval<err_type>())) const
    {
        using return_type = decltype(uf(std::declval<err_type>()));

        return match(
            [&f, &uf](const ok_type& v) {
                return f(v).match([](const hfl::ok<return_type>& v){ return v.m_value; }, [&uf](const err_type& e){return uf(e);});
            },
            [&uf](const err_type& e) {
                return uf(e);
            });
    }

    template<typename EFunc>
    auto map_err(EFunc&& f) -> rs_result<T, decltype(f(std::declval<E>()))> const
    {
        using return_type = decltype(f(std::declval<E>()));
        return match(
            [](const ok_type& v) {
                return rs_result<T, return_type>{v};
            },
            [&f](const err_type& e) {
                return rs_result<T, return_type>{err<return_type>{f(e.m_value)}};
            });
    }

    template<typename Func>
    auto and_then(Func&& f) const noexcept -> decltype(f(std::declval<T>()))
    {
        return match(
            [&f](const ok_type& v) {
                return f(v.m_value);
            },
            [](const err_type& e) {
                return decltype(f(std::declval<T>())){e};
            }); 
    }

    template<typename Func>
    auto or_else(Func&& f) const noexcept -> decltype(f(std::declval<E>()))
    {
        return match(
            [](const ok_type& v) {
                return decltype(f(std::declval<E>())){v};
            },
            [&f](const err_type& e) {
                return f(e.m_value);
            }); 
    }

    constexpr auto as_mut() noexcept -> rs_result<row_ok_type&, row_err_type&>
    {
        return match(
            [](ok_type& v) {
                return rs_result<row_ok_type&, row_err_type&>{ hfl::ok<row_ok_type&>{v.m_value}};
            },
            [](err_type& e) {
                return rs_result<row_ok_type&, row_err_type&>{ hfl::err<row_err_type&>{e.m_value}};
            });
    }

    constexpr auto as_ref() const noexcept -> rs_result<const row_ok_type&, const row_err_type&>
    {
        return match(
            [](const ok_type& v) {
                return rs_result<const row_ok_type&, const row_err_type&>{ hfl::ok<const row_ok_type&>{v.m_value}};
            },
            [](const err_type& e) {
                return rs_result<const row_ok_type&, const row_err_type&>{ hfl::err<const row_err_type&>{e.m_value}};
            });
    }

    constexpr auto copied() const noexcept -> rs_result<row_ok_type, row_err_type>
    {
        return match(
            [](const ok_type& v) {
                return rs_result<row_ok_type, row_err_type>{ hfl::ok<row_ok_type>{v.m_value}};
            },
            [](const err_type& e) {
                return rs_result<row_ok_type, row_err_type>{ hfl::err<row_err_type>{e.m_value}};
            });
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
constexpr auto mbind(const rs_result<FT, FE>& res, Func&& f) -> decltype(f(std::declval<FT>()))
{
    if (res.is_ok())
    {
        return f(res.inter_unwrap_ok().m_value);
    }
    else
    {
        return decltype(f(std::declval<FT>())){res.inter_unwrap_err()};
    }
}

template<typename T, typename E, typename Func>
constexpr decltype(auto) operator|(rs_result<T, E> res, Func&& f)
{
    return mbind<T, E, Func>(res, std::forward<Func>(f));
}

template<typename T, typename E, typename... Funcs>
constexpr decltype(auto) pipeline(const rs_result<T, E>& res, Funcs&&... f)
{
    return ((res | ... | f));
}

template<typename T, typename E, typename OkFunc, typename ErrFunc>
constexpr decltype(auto) match(const rs_result<T, E>& res, OkFunc&& val_func, ErrFunc&& err_func)
{
    return res.match(std::forward<OkFunc>(val_func), std::forward<ErrFunc>(err_func));
}

} // namespace hfl