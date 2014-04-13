#pragma once

extern "C" {
#include <lua.h>
}

#include "lua_nil.h"

#include <type_traits>

namespace lua {

template<typename Type>
inline
typename std::enable_if<std::is_integral<Type>::value && !std::is_same<Type,bool>::value>::type
push( ::lua_State* l_, Type value_ ) {
    lua_pushinteger( l_, value_ );
}

template<typename Type>
inline
typename std::enable_if<std::is_floating_point<Type>::value>::type
push( ::lua_State* l_, Type value_ ) {
    lua_pushnumber( l_, value_ );
}

inline void push( ::lua_State* l_, bool value_ ) {
    lua_pushboolean( l_, value_ ? 1 : 0 );
}

inline void push( ::lua_State* l_, nil ) { lua_pushnil( l_ ); }

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