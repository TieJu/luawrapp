#pragma once

#include <type_traits>
#include <tuple>

#include "lua_nil.h"

namespace lua {
template<typename Type>
inline bool is( ::lua_State* l_, int index_ ) {
    return lua::type_trait<std::decay_t<Type>>::is( l_, index_ );
}

namespace detail {
template<typename First>
inline bool is_r( ::lua_State* l_, int index_, int /*step_*/ ) {
    return is<First>( l_, index_ );
}
template<typename First, typename... Args>
inline
typename std::enable_if<sizeof...(Args) != 0,bool>::type
is_r( ::lua_State* l_, int index_, int step_ ) {
    return is<First>( l_, index_ ) && is_r<Args...>( l_, index_ + step_, step_ );
}
}

template<typename... Args>
inline bool is( ::lua_State* l_, int index_, int step_ ) {
    return detail::is_r<Args...>( l_, index_, step_ );
}

}