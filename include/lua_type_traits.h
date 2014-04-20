#pragma once

#include <type_traits>

#include "lua_nil.h"

namespace lua {
template<typename Type, typename Enable = void>
struct type_trait {};

template<>
struct type_trait<bool> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TBOOLEAN == lua_type( l_, index_ );
    }

    static bool to( ::lua_State* l_, int index_ ) {
        return lua_toboolean( l_, index_ ) != 0;
    }

    static void push( ::lua_State* l_, bool value_ ) {
        lua_pushboolean( l_, value_ );
    }
};

template<>
struct type_trait<nil> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TNIL == lua_type( l_, index_ );
    }

    static nil to( ::lua_State* /*l_*/, int /*index_*/ ) {
        return {};
    }

    static void push( ::lua_State* l_, nil ) {
        lua_pushnil( l_ );
    }
};

template<>
struct type_trait<const char*> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TSTRING == lua_type( l_, index_ );
    }

    static const char* to( ::lua_State* l_, int index_ ) {
        return lua_tostring( l_, index_ );
    }

    static void push( ::lua_State* l_, const char* str_ ) {
        lua_pushstring( l_, str_ );
    }
};

template<>
struct type_trait<lua_CFunction> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static lua_CFunction to( ::lua_State* l_, int index_ ) {
        return lua_tocfunction( l_, index_ );
    }

    static void push( ::lua_State* l_, lua_CFunction cf_ ) {
        lua_pushcfunction( l_, cf_ );
    }
};

template<typename Type>
struct type_trait<Type, typename std::enable_if<std::is_floating_point<Type>::value>::type> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TNUMBER == lua_type( l_, index_ );
    }

    static Type to( ::lua_State* l_, int index_ ) {
        return static_cast<Type>( lua_tonumber( l_, index_ ) );
    }

    static void push( ::lua_State* l_, Type value_ ) {
        lua_pushnumber( l_, value_ );
    }
};

template<typename Type>
struct type_trait<Type, typename std::enable_if<std::is_integral<Type>::value && !std::is_same<bool,Type>::value>::type> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TNUMBER == lua_type( l_, index_ );
    }

    static Type to( ::lua_State* l_, int index_ ) {
        return static_cast<Type>( lua_tointeger( l_, index_ ) );
    }

    static void push( ::lua_State* l_, Type value_ ) {
        lua_pushinteger( l_, value_ );
    }
};
}