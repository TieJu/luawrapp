#pragma once

#include <type_traits>

extern "C" {
#include <lua.h>
}

#include "lua_nil.h"

namespace lua {
template<typename Type>
inline
typename std::enable_if<std::is_integral<Type>::value && !std::is_same<Type,bool>::value, Type>::type
to( ::lua_State* l_, int index_ ) {
    return static_cast<Type>( lua_tointeger( l_, index_ ) );
}

template<typename Type>
inline
typename std::enable_if<std::is_same<Type, bool>::value, Type>::type
to( ::lua_State* l_, int index_ ) {
    return 0 != lua_toboolean( l_, index_ );
}

template<typename Type>
inline
typename std::enable_if<std::is_floating_point<Type>::value, Type>::type
to( ::lua_State* l_, int index_ ) {
    return static_cast<Type>( lua_tonumber( l_, index_ ) );
}
}