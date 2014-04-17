#pragma once

#include <type_traits>
#include <tuple>

#include "lua_nil.h"

namespace lua {
template<typename Type>
inline Type to( ::lua_State* l_, int index_ ) {
    return lua::type_trait<std::decay_t<Type>>::to( l_, index_ );
}

namespace detail {
template<typename First>
inline std::tuple<First> to_r( ::lua_State* l_, int index_, int step_ ) {
    return std::make_tuple( to<First>( l_, index_ ) );
}
template<typename First, typename... Args>
inline
typename std::enable_if<sizeof...(Args) != 0,std::tuple<First, Args...>>::type
to_r( ::lua_State* l_, int index_, int step_ ) {
    return std::tuple_cat( std::make_tuple( to<First>( l_, index_ ) )
                         , to_r<Args...>( l_, index_ + step_, step_ ) );
}
}

template<typename... Args>
inline std::tuple<Args...> to( ::lua_State* l_, int index_, int step_ ) {
    return detail::to_r<Args...>( l_, index_, step_ );
}
}