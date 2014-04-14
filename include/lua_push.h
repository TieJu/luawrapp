#pragma once

extern "C" {
#include <lua.h>
}

#include "lua_nil.h"
#include "lua_type_traits.h"

namespace lua {
template<typename Type>
inline void push( ::lua_State* l_, Type&& value_ ) {
    type_trait<std::decay_t<Type>>::push( l_, std::forward<Type>( value_ ) );
}

namespace detail {
inline void push_r( ::lua_State* l_ ) {}
template<typename First,typename... Args>
inline void push_r( ::lua_State* l_, First&& first_, Args&&... args_ ) {
    push( l_, std::forward<First>( first_ ) );
    push_r( l_, std::forward<Args>( args_ )... );
}
}

template<typename... Args>
inline void push( ::lua_State* l_, Args&&... args_ ) {
    lua_checkstack( l_, sizeof...( Args ) );
    detail::push_r( l_, std::forward<Args>( args_ )... );
}
}