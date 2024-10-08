#pragma once
#include "hfl_concept.hpp"
#include <functional>
#include <tuple>

namespace hfl
{

template<typename Func>
using function = std::function<Func>;

template<size_t Rem, typename Sig>
struct curried_helper
{
};

template<size_t Rem, typename Ret, typename... Args>
struct curried_helper<Rem, Ret(Args...)>
{
    using args_tuple_type = std::tuple<Args...>;
    using function_type = function<Ret(Args...)>;
    using arg_type = std::tuple_element_t<std::tuple_size_v<args_tuple_type> - Rem, args_tuple_type>;

    constexpr curried_helper(function_type& f, args_tuple_type& args) : m_f(f), m_args(args)
    {
    }

    constexpr auto operator()(const std::remove_cvref_t<arg_type>& arg)
    {
        std::get<std::tuple_size_v<args_tuple_type> - Rem>(m_args) = arg;
        return curried_helper<Rem - 1, Ret(Args...)>(m_f, m_args);
    }
    function_type& m_f;
    args_tuple_type& m_args;
};

template<typename Ret, typename... Args>
struct curried_helper<1, Ret(Args...)>
{
    using args_tuple_type = std::tuple<Args...>;
    using function_type = function<Ret(Args...)>;
    using arg_type = std::tuple_element_t<std::tuple_size_v<args_tuple_type> - 1, args_tuple_type>;

    constexpr curried_helper(function_type& f, args_tuple_type& args) : m_f(f), m_args(args)
    {
    }

    constexpr decltype(auto) operator()(const std::remove_cvref_t<arg_type>& arg)
    {
        std::get<std::tuple_size_v<args_tuple_type> - 1>(m_args) = arg;
        return std::apply(m_f, m_args);
    }

private:
    function_type& m_f;
    args_tuple_type& m_args;
};

template<typename Sig>
struct curried
{
};

template<typename Ret, typename... Args>
struct curried<Ret(Args...)>
{
    using args_tuple_type = std::tuple<Args...>;
    using first_arg_type = std::tuple_element_t<0, args_tuple_type>;
    using function_type = function<Ret(Args...)>;

    template<typename Func>
        requires callable_with_tuple_args<Func, args_tuple_type>
    constexpr curried(Func&& f) : m_f(std::forward<Func>(f)), m_tuple_args()
    {
    }

    constexpr decltype(auto) operator()(first_arg_type arg)
    {
        if constexpr (std::tuple_size_v<args_tuple_type> == 1)
        {
            auto& arg_val = std::get<0>(m_tuple_args);
            arg_val = arg;
            return std::apply(m_f, m_tuple_args);
        }
        else
        {
            auto& arg_val = std::get<0>(m_tuple_args);
            arg_val = arg;
            return curried_helper<std::tuple_size_v<args_tuple_type> - 1, Ret(Args...)>{m_f, m_tuple_args};
        }
    }

private:
    function_type m_f;
    args_tuple_type m_tuple_args;
};


} // namespace hfl