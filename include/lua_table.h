#pragma once

#include <type_traits>

namespace lua {
template<typename Key, typename Value>
inline void set_table( ::lua_State* l_, int table_id_, Key key_, Value value_ ) {
    ::lua::push( l_, std::forward<Key>( key_ ) );
    ::lua::push( l_, std::forward<Value>( value_ ) );
    ::lua_settable( l_, table_id_ );
}

template<typename Key>
inline void get_table( ::lua_State* l_, int table_id_, Key key_ ) {
    ::lua::push( l_, std::forward<Key>( key_ ) );
    ::lua_gettable( l_, table_id_ );
}

template<typename Key, typename Value>
inline void raw_set( ::lua_State* l_, int table_id_, Key key_, Value value_ ) {
    ::lua::push( l_, std::forward<Key>( key_ ) );
    ::lua::push( l_, std::forward<Value>( value_ ) );
    ::lua_rawset( l_, table_id_ );
}

template<typename Key>
inline void raw_get( ::lua_State* l_, int table_id_, Key key_ ) {
    ::lua::push( l_, std::forward<Key>( key_ ) );
    ::lua_rawget( l_, table_id_ );
}
}