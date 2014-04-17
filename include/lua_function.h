#pragma once


#include "tools_v_seq.h"

#include "lua_type_traits.h"

namespace lua {
template<typename Ret, typename... Args>
struct type_trait<Ret( *)( Args... )> {
    typedef Ret( *type )( Args... );
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static type to( ::lua_State* l_, int index_ ) {
        //return lua_tonumber( l_, index_ ) != 0;
        return nullptr;
    }

    static void push( ::lua_State* l_, type value_ ) {
        lua_pushlightuserdata( l_, value_ );
        lua_pushcclosure( l_, &proxy, 1 );
    }
    
    template<typename Ret, typename Func, typename Tup, int... Seq>
    static Ret invoke( Func func_, Tup&& tup_, tools::seq<Seq...> ) {
        return func_( std::get<Seq>( tup_ )... );
    }

    static int proxy( ::lua_State* l_ ) {
        auto func_ = static_cast<type>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        auto params = lua::to<Args...>( l_, 1, 1 );
        return ::lua::push( l_, invoke<Ret>( func_, std::move( params ), typename tools::gen_seq<sizeof...( Args )>::type {} ) );
    }
};

template<typename Ret>
struct type_trait<Ret( *)( void )> {
    typedef Ret( *type )( void );
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static type to( ::lua_State* l_, int index_ ) {
        //return lua_tonumber( l_, index_ ) != 0;
        return nullptr;
    }

    static void push( ::lua_State* l_, type value_ ) {
        lua_pushlightuserdata( l_, value_ );
        lua_pushcclosure( l_, &proxy, 1 );
    }

    static int proxy( ::lua_State* l_ ) {
        auto func_ = static_cast<type>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        return ::lua::push( l_, func_() );
    }
};

template<typename... Args>
struct type_trait<void( *)( Args... )> {
    typedef void( *type )( Args... );
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static type to( ::lua_State* l_, int index_ ) {
        //return lua_tonumber( l_, index_ ) != 0;
        return nullptr;
    }

    static void push( ::lua_State* l_, type value_ ) {
        lua_pushlightuserdata( l_, value_ );
        lua_pushcclosure( l_, &proxy, 1 );
    }

    template<typename Func, typename Tup, int... Seq>
    static void invoke( Func func_, Tup&& tup_, tools::seq<Seq...> ) {
        func_( std::get<Seq>( tup_ )... );
    }

    static int proxy( ::lua_State* l_ ) {
        auto func_ = static_cast<type>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        auto params = lua::to<Args...>( l_, 1, 1 );
        invoke( func_, std::move( params ), typename tools::gen_seq<sizeof...( Args )>::type {} );
        return 0;
    }
};

template<>
struct type_trait<void( *)( void )> {
    typedef void( *type )( void );
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static type to( ::lua_State* l_, int index_ ) {
        //return lua_tonumber( l_, index_ ) != 0;
        return nullptr;
    }

    static void push( ::lua_State* l_, type value_ ) {
        lua_pushlightuserdata( l_, value_ );
        lua_pushcclosure( l_, &proxy, 1 );
    }

    static int proxy( ::lua_State* l_ ) {
        auto func_ = static_cast<type>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        func_();
        return 0;
    }
};

template<typename... Args>
struct type_trait<int( *)( ::lua_State*, Args... )> {
    typedef int( *type )( ::lua_State*, Args... );
    static bool is( ::lua_State* l_, int index_ ) {
        return LUA_TFUNCTION == lua_type( l_, index_ );
    }

    static type to( ::lua_State* l_, int index_ ) {
        //return lua_tonumber( l_, index_ ) != 0;
        return nullptr;
    }

    static void push( ::lua_State* l_, type value_ ) {
        lua_pushlightuserdata( l_, value_ );
        lua_pushcclosure( l_, &proxy, 1 );
    }

    template<typename Func, typename Tup, int... Seq>
    static int invoke( Func func_, ::lua_State* l_, Tup&& tup_, tools::seq<Seq...> ) {
        return func_( l_, std::get<Seq>( tup_ )... );
    }

    static int proxy( ::lua_State* l_ ) {
        auto func_ = static_cast<type>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        auto params = lua::to<Args...>( l_, 1, 1 );
        return invoke( func_, l_, std::move( params ), typename tools::gen_seq<sizeof...( Args )>::type {} );
    }
};
}