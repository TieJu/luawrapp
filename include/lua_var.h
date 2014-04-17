#pragma once

extern "C" {
#include <lua.h>
}

#include <utility>
#include <cassert>

#include "lua_type_traits.h"
#include "lua_push.h"
#include "lua_to.h"

namespace lua {
template<typename State>
class shared_var {
    State   _state;

    void set_ref( int _pos, bool remove_ = false ) {
        _state.push_light_user_data( this );
        _state.push_value( _pos );
        _state.raw_set( LUA_REGISTRYINDEX );
        if ( remove_ ) {
            _state.remove( _pos );
        }
    }

    void get_ref() {
        _state.push_light_user_data( this );
        _state.raw_get( LUA_REGISTRYINDEX );
    }

    void unset_ref() {
        _state.push_light_user_data( this );
        _state.push_nil();
        _state.raw_set( LUA_REGISTRYINDEX );
    }

public:
    shared_var( State state_, int index_ ) : _state { state_ } { set_ref( index_, false ); /* do not remove value from stack */ }
    template<typename Type>
    shared_var( State state_, Type value_ ) : _state { state_ } { _state.push( std::forward<Type>( value_ ) ); set_ref( _state.get_top(), true ); }
    ~shared_var() { unset_ref(); }

    template<typename Type>
    void set( Type value_ ) {
        _state.push( std::forward<Type>( value_ ) );
        set_ref( _state.get_top(), true );
    }

    template<typename OtherState>
    void set( const stack_var<OtherState>& svar_ ) {
        assert( svar_.state() == _state );
        svar_.push_value();
        set_ref( _state.get_top(), true );
    }

    template<typename Type>
    shared_var<State>& operator=( Type value_ ) {
        set( std::forward<Type>( value_ ) );
        return *this;
    }

    void push_value() {
        get_ref();
    }

    stack_var<State> push_to_stack() {
        get_ref();
        return stack_var<State> { _state, _state.get_top() };
    }
};

template<typename State>
shared_var<State> stack_var<State>::share() {
    return shared_var<State> { _state, _pos };
}
}