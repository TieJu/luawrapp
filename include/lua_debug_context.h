#pragma once

#include <unordered_map>
#include <functional>

namespace lua {
    inline void dump_stack_entry( ::lua_State* l_, int index_ );
inline void dump_table( ::lua_State* l_, int index_ ) {
    printf( "[%d] %s\n", index_, lua_typename( l_, LUA_TTABLE ) );

    lua_pushnil( l_ );
    while ( lua_next( l_, index_ ) ) {
        puts( "{\n" );
        puts( "  " );
        dump_stack_entry( l_, lua_gettop(l_)-2+1 );
        puts( " =\n  " );
        dump_stack_entry( l_, lua_gettop( l_ ) - 1+1 );
        puts( "}\n" );
        lua_pop( l_, 1 );
    }
}
inline void dump_stack_entry( ::lua_State* l_, int index_ ) {
    auto type = lua_type( l_, index_ );
    switch ( type ) {
    case LUA_TNONE: printf( "[%d] %s\n", index_, lua_typename( l_, type ) ); break;
    case LUA_TNIL: printf( "[%d] %s\n", index_, lua_typename( l_, type ) ); break;
    case LUA_TBOOLEAN: printf( "[%d] %s = %s\n", index_, lua_typename( l_, type ), lua_toboolean( l_, index_ ) ? "true" : "false" ); break;
    case LUA_TLIGHTUSERDATA: printf( "[%d] %s = %p\n", index_, lua_typename( l_, type ), lua_topointer( l_, index_ ) ); break;
    case LUA_TNUMBER: printf( "[%d] %s = %f\n", index_, lua_typename( l_, type ), lua_tonumber( l_, index_ ) ); break;
    case LUA_TSTRING: printf( "[%d] %s = %s\n", index_, lua_typename( l_, type ), lua_tostring( l_, index_ ) ); break;
    case LUA_TTABLE: if ( index_ > 0 ) { dump_table( l_, index_ ); } else { printf( "[%d] %s\n", index_, lua_typename( l_, type ) ); }break;
    case LUA_TFUNCTION: printf( "[%d] %s = %p\n", index_, lua_typename( l_, type ), lua_topointer( l_, index_ ) ); break;
    case LUA_TUSERDATA: printf( "[%d] %s = %p\n", index_, lua_typename( l_, type ), lua_topointer( l_, index_ ) ); break;
    case LUA_TTHREAD: printf( "[%d] %s = %p\n", index_, lua_typename( l_, type ), lua_topointer( l_, index_ ) ); break;
    }
}

inline void dump_stack( ::lua_State *l_ ) {
    puts( "lua stack >>>\n" );
    auto top = lua_gettop( l_ );
    for ( int i = 1; i <= top; ++i ) {
        dump_stack_entry( l_, i );
    }
    puts( "<<< lua stack\n" );
}
template<typename Context>
struct debug_context {
    Context _ctx;

    typedef std::function<void( ::lua_State*, int )> debug_clb;
    struct debug_info {
        size_t                              _ref { 0 };
        std::unordered_map<int, debug_clb>  on_line_break;
    };

    static debug_info* get_debug_info( ::lua_State* l_ ) {
        if ( !l_ ) {
            return nullptr;
        }
        lua_pushstring( l_, "__debug_state__" );
        lua_gettable( l_, LUA_REGISTRYINDEX );
        return ( debug_info*)lua_topointer( l_, -1 );
    }

    static void hook( lua_State *l_, lua_Debug *ar_ ) {
        auto info = get_debug_info( l_ );
        switch ( ar_->event ) {
        case LUA_HOOKCALL: {
            lua_getinfo( l_, "nSlu", ar_ );
            } break;
        case LUA_HOOKLINE: {
            auto ref = info->on_line_break.find( ar_->currentline );
            if ( ref != end( info->on_line_break ) ) {
                ref->second( l_, ar_->currentline );
            }
            } break;
        case LUA_HOOKRET:
            break;
        case LUA_HOOKTAILRET:
            break;
        }
    }

    void register_handler() {
        auto l_ = _ctx.get();
        auto info = std::make_unique<debug_info>();
        lua_pushstring( l_, "__debug_state__" );
        lua_pushlightuserdata( l_, info.release() );
        lua_settable( l_, LUA_REGISTRYINDEX );
        lua_sethook( l_, hook, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0 );
    }

    void unregister_handler() {
        auto l_ = _ctx.get();
        lua_sethook( l_, nullptr, 0, 0 );
        lua_pushstring( l_, "__debug_state__" );
        lua_gettable( l_, LUA_REGISTRYINDEX );
        std::unique_ptr<debug_info> info { (debug_info*)lua_topointer( l_, -1 ) };
        lua_pushstring( l_, "__debug_state__" );
        lua_pushnil( l_ );
        lua_settable( l_, LUA_REGISTRYINDEX );
    }

    void add_ref() {
        auto info = get_debug_info( _ctx.get() );
        if ( !info ) {
            register_handler();
            info = get_debug_info( _ctx.get() );
        }
        ++info->_ref;
    }

    void sub_ref() {
        auto info = get_debug_info( _ctx.get() );
        if ( info ) {
            if ( !--info->_ref ) {
                unregister_handler();
            }
        }
    }

public:
    debug_context( Context ctx_ ) : _ctx { std::forward<Context>( ctx_ ) } { add_ref(); }
    ~debug_context() { sub_ref(); }

    debug_context( const debug_context & other_ )
        : _ctx { other_._ctx } {
        add_ref();
    }
    debug_context& operator=( const debug_context & other_ ) {
        sub_ref();
        _ctx = other_._ctx;
        add_ref();
        return *this;
    }

    debug_context( debug_context&& other_ )
        : _ctx { std::move( other_._ctx ) } {
    }

    debug_context& operator=( debug_context&& other_ ) {
        sub_ref();
        _ctx = std::move( other_._ctx );
        return *this;
    }

    template<typename Callback>
    void add_line_break( int line_, Callback clb_ ) {
        get_debug_info( _ctx.get() )->on_line_break[line_] = clb_;
    }
};
}