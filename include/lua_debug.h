#pragma once

#include <bitset>

namespace lua {
template<typename Derived>
struct debug {

    struct debug_event {// only to put it into a namespace
        enum {
            call = LUA_HOOKCALL,
            ret = LUA_HOOKRET,
            line = LUA_HOOKLINE,
            count = LUA_HOOKCOUNT,
            tail_ret = LUA_HOOKTAILRET
        };
    };

    typedef std::bitset<5> debug_mask;

    bool get_stack( int level_, lua_Debug& dbg_ ) {
        return 0 != lua_getstack( static_cast<Derived*>( this )->get(), level_, &dbg_ );
    }

    bool get_info( const char* what_, lua_Debug& dbg_ ) {
        return 0 != lua_getinfo( static_cast<Derived*>( this )->get(), what_, dbg_ );
    }

    const char* get_local( const lua_Debug& dbg_, int n_ ) {
        return lua_getlocal( static_cast<Derived*>( this )->get(), &dbg_, n_ );
    }

    const char* set_local( const lua_Debug& dbg_, int n_ ) {
        return lua_setlocal( static_cast<Derived*>( this )->get(), &dbg_, n_ );
    }

    const char* get_upvalue( int func_, int n_ ) {
        return lua_getupvalue( static_cast<Derived*>( this )->get(), func_, n_ );
    }

    const char* set_upvalue( int func_, int n_ ) {
        return lua_setupvalue( static_cast<Derived*>( this )->get(), func_, n_ );
    }

    int set_hook( lua_Hook hook_, debug_mask mask_, int count_ ) {
        return lua_sethook( static_cast<Derived*>( this )->get(), hook_, mask_.to_ulong(), count_ );
    }

    lua_Hook get_hook() {
        return lua_gethook( static_cast<Derived*>( this )->get() );
    }

    debug_mask get_hook_mask() {
        return { lua_gethookmask( static_cast<Derived*>( this )->get() ) };
    }

    int get_hook_count() {
        return lua_gethookcount( static_cast<Derived*>( this )->get() );
    }
};
}