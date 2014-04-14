#pragma once

extern "C" {
#include <lua.h>
}

#include "lua_nil.h"
#include "lua_type_traits.h"

#include "tools_v_seq.h"

namespace lua {
template<typename Type>
inline size_t push( ::lua_State* l_, Type&& value_ ) {
    type_trait<std::decay_t<Type>>::push( l_, std::forward<Type>( value_ ) );
    return 1;
}

namespace detail {
template<typename First>
inline void push_r( ::lua_State* l_, First&& first_ ) {
    push( l_, std::forward<First>( first_ ) );
}
template<typename First,typename... Args>
inline 
typename std::enable_if<sizeof...( Args ) != 0>::type
push_r( ::lua_State* l_, First&& first_, Args&&... args_ ) {
    push( l_, std::forward<First>( first_ ) );
    push_r( l_, std::forward<Args>( args_ )... );
}
}

template<typename... Args>
inline size_t push( ::lua_State* l_, Args&&... args_ ) {
    lua_checkstack( l_, sizeof...( Args ) );
    detail::push_r( l_, std::forward<Args>( args_ )... );
    return sizeof...( Args );
}

namespace detail {
template<typename Tup, int... Seq>
inline void push_t( ::lua_State* l_, Tup&& tup_, tools::seq<Seq...> ) {
    push( l_, std::get<Seq>( tup_ )... );
}
}

template<typename... Args>
inline void push( ::lua_State* l_, const std::tuple<Args...>& tupl_ ) {
    lua_checkstack( l_, sizeof...( Args ) );
    detail::push_t( l_, tupl_, typename tools::gen_seq<sizeof...( Args )>::type {} );
    return sizeof...( Args );
}

template<typename... Args>
inline size_t push( ::lua_State* l_, std::tuple<Args...>&& tupl_ ) {
    lua_checkstack( l_, sizeof...( Args ) );
    detail::push_t( l_, std::forward<std::tuple<Args...>>( tupl_ ), typename tools::gen_seq<sizeof...( Args )>::type {} );
    return sizeof...( Args );
}
}