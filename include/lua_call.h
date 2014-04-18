#pragma once

#include <tuple>

namespace lua {
template<typename... RetArgs, typename... InArgs>
inline
typename std::enable_if<sizeof...(RetArgs ) != 0, std::tuple<RetArgs...>>::type
call( lua_State* l_, InArgs... args_ ) {
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_call( l_, sizeof...( InArgs ), sizeof...( RetArgs ) );
    return lua::to<RetArgs...>( l_, top, 1 );
}
template<typename... InArgs>
inline void call( lua_State* l_, InArgs... args_ ) {
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_call( l_, sizeof...( InArgs ), 0 );
}

template<typename... RetArgs, typename... InArgs>
inline
typename std::enable_if<sizeof...( RetArgs ) != 0, std::tuple<RetArgs...>>::type
pcall( lua_State* l_, int error_handler_, InArgs... args_ ) {
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_pcall( l_, sizeof...( InArgs ), sizeof...( RetArgs ), int error_handler_ );
    return lua::to<RetArgs...>( l_, top, 1 );
}

template<typename... InArgs>
inline void pcall( lua_State* l_, int error_handler_, InArgs... args_ ) {
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_pcall( l_, sizeof...( InArgs ), 0, int error_handler_ );
}

template<typename... RetArgs, typename... InArgs>
inline
typename std::enable_if<sizeof...( RetArgs ) != 0, std::tuple<RetArgs...>>::type
pcall( lua_State* l_, lua_CFunction error_handler_, InArgs... args_ ) {
    lua_pushcfunction( l_, error_handler_ );
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_pcall( l_, sizeof...( InArgs ), sizeof...( RetArgs ), top + 1 );
    return lua::to<RetArgs...>( l_, top, 1 );
}

template<typename... InArgs>
inline void pcall( lua_State* l_, lua_CFunction error_handler_, InArgs... args_ ) {
    lua_pushcfunction( l_, error_handler_ );
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_pcall( l_, sizeof...( InArgs ), 0, top + 1 );
}
}