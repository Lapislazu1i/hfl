#pragma once
#include <tuple>

namespace hfl
{

template<typename Func, typename TupleArgsType>
concept callable_with_tuple_args = requires(Func f, TupleArgsType args) { std::apply(f, args); };

}