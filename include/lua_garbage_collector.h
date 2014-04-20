#pragma once

namespace lua {
template<typename Derived>
struct garbage_collector {

    enum class gc_param {
        stop = LUA_GCSTOP,
        restart = LUA_GCRESTART,
        collect = LUA_GCCOLLECT,
        count = LUA_GCCOUNT,
        count_bytes = LUA_GCCOUNTB,
        step = LUA_GCSTEP,
        pause = LUA_GCSETPAUSE,
        step_mul = LUA_GCSETSTEPMUL
    };
    int gc( gc_param param_, int data_ ) {
        return lua_gc( static_cast<Derived*>( this )->get(), int( param_ ), data_ );
    }

    int get_gc_count() {
        return lua_getgccount( static_cast<Derived*>( this )->get() );
    }
};
}