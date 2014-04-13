#pragma once

extern "C" {
#include <lua.h>
}

#include <string>

namespace lua {
inline void push( ::lua_State* l_, const std::string& str_ ) {
    lua_pushlstring( l_, str_.c_str(), str_.length() );
}

template<typename Type>
inline
typename std::enable_if<std::is_same<std::decay_t<Type>, std::string>::value, std::decay_t<Type>>::type
to( ::lua_State* l_, int index_ ) {
    size_t l;
    auto c = lua_tolstring( l_, index_, &l );
    return { c, l };
}
}