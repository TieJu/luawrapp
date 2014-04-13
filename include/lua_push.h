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
    lua_pushvalue( l_, value_ );
}

inline void push( ::lua_State* l_, bool value_ ) {
    lua_pushboolean( l_, value_ ? 1 : 0 );
}

inline void push( ::lua_State* l_, nil ) { lua_pushnil( l_ ); }
inline void push( ::lua_State* l_, nullptr_t ) { static_assert( false, "tryed to push nullptr" ); }
}