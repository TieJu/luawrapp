#pragma once

#include "lua_type_traits.h"

namespace lua {
struct light_user_data {
    void* _ptr;
};

template<>
struct type_trait<light_user_data> {
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TLIGHTUSERDATA == lua_type( l_, index_ );
    }

    static light_user_data to( ::lua_State* l_, int index_ ) {
        return { lua_touserdata( l_, index_ ) };
    }

    static void push( ::lua_State* l_, light_user_data value_ ) {
        lua_pushlightuserdata( l_, value_._ptr );
    }
};
}