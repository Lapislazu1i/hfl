#pragma once
#include <tuple>
#include <mutex>
#include <map>

namespace hfl
{

class null_param{};

template<typename Sig, typename F>
class memoize_helper;

template<typename Ret, typename... Args, typename F>
class memoize_helper<Ret(Args...), F>
{
public:
    template<typename Function>
    constexpr memoize_helper(Function&& f, null_param)
        : m_f(f) {}

    constexpr memoize_helper(const memoize_helper& other)
        : m_f(other.f){}

    template<typename... InnerArgs>
    Ret operator()(InnerArgs&&... args) const
    {
        std::unique_lock<std::mutex> lock(m_cache_mutex);
        const auto args_tuple = std::make_tuple(args...);
        const auto cached = m_cache.find(args_tuple);
        if(cached != m_cache.end())
        {
            return cached->second;
        }
        auto&& result = m_f(std::forward<InnerArgs>(args)...);
        m_cache[args_tuple] = result;
        return result;
    }

private:
    using function_type = F;
    using args_tuple_type = std::tuple<std::decay_t<Args>...>;
    function_type m_f;
    mutable std::map<args_tuple_type, Ret> m_cache{};
    mutable std::mutex m_cache_mutex{};
};

template<typename Sig, typename F>
class recursive_memoize_helper;

template<typename Ret, typename... Args, typename F>
class recursive_memoize_helper<Ret(Args...), F>
{
public:
    template<typename Function>
    constexpr recursive_memoize_helper(Function&& f, null_param)
        : m_f(f) {}

    constexpr recursive_memoize_helper(const recursive_memoize_helper& other)
        : m_f(other.f){}

    template<typename... InnerArgs>
    Ret operator()(InnerArgs&&... args) const
    {
        std::unique_lock<std::recursive_mutex> lock(m_cache_mutex);
        const auto args_tuple = std::make_tuple(args...);
        const auto cached = m_cache.find(args_tuple);
        if(cached != m_cache.end())
        {
            return cached->second;
        }
        auto&& result = m_f(*this, std::forward<InnerArgs>(args)...);
        m_cache[args_tuple] = result;
        return result;
    }

private:
    using function_type = F;
    using args_tuple_type = std::tuple<std::decay_t<Args>...>;
    function_type m_f;
    mutable std::map<args_tuple_type, Ret> m_cache{};
    mutable std::recursive_mutex m_cache_mutex{};
};


template<typename Sig, typename F>
constexpr recursive_memoize_helper<Sig, std::decay_t<F>> make_recursive_memo(F&& f)
{
    return {std::forward<F>(f), null_param{}};
}

template<typename Sig, typename F>
constexpr memoize_helper<Sig, std::decay_t<F>> make_memo(F&& f)
{
    return {std::forward<F>(f), null_param{}};
}

}