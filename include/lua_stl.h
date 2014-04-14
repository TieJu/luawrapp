#pragma once

extern "C" {
#include <lua.h>
}

#include <string>

namespace lua {
template<typename Type>
struct type_trait<Type, typename std::enable_if<std::is_same<typename std::decay<Type>::type, std::string>::value>::type> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TSTRING == lua_type( l_, index_ );
    }

    static std::string to( ::lua_State* l_, int index_ ) {
        size_t len;
        auto c_str = lua_tolstring( l_, index_, &len );
        return { c_str, len };
    }

    static void push( ::lua_State* l_, const std::string& str_ ) {
        lua_pushlstring( l_, str_.c_str(), str_.length() );
    }
};
}