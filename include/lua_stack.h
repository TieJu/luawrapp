#pragma once

extern "C" {
#include <lua.h>
}

#include <cassert>

namespace lua {
template<typename State>
class stack_slot {
    State   _state;
    int     _index;

public:
    stack_slot( State state_, int index_ ) : _state { state_ }, _index { state_.abs_stack_index( index_ ) } {}
    ~stack_slot() { if ( is_top() ) { _state.pop(); } }

    const State& state() const { return _state; }
    State& state() { return _state; }
    int index() const { return _pos; }
    int set_index( int index_ ) { _index = index_; }
    int move_index( int index_ ) { _index += index_; }
    int type() const { return _state.type( _pos ); }

    template<typename Type>
    stack_var<State> get_table( Type&& index_ ) {
        _state.push( std::forward<Type>( index_ ) );
        _state.get_table( _pos );
        return stack_var<State> { _state, _state.get_top() };
    }

    template<typename Index, typename Type>
    void set_table( Index&& index_, Type&& value_ ) {
        _state.push( std::forward<Index>( index_ ), std::forward<Type>( value_ ) );
        _state.set_table( _pos );
    }

    void replace() {
        _state.replace( _pos );
    }

    void push_value() const {
        _state.push_value( _pos );
    }

    stack_slot<State> push() const {
        push_value();
        return stack_slot<State>{_state, _state.get_top()};
    }

    void set( const stack_slot<State>& other_ ) {
        if ( _state == other_._state ) {
            _state.push_value( other_._pos );
            _state.replace( _pos );
        } else {
            _state.push_value( other_._pos );
            _state.xmove_from( other_._state, 1 );
            _state.replace( _pos );
        }
    }

    template<typename Type>
    void set( Type value_ ) {
        if ( is_top() ) {
            _state.pop();
            _state.push( std::move( value_ ) );
        } else {
            _state.push( std::move( value_ ) );
            _state.replace( _pos );
        }
    }

    template<typename Type>
    Type to() const {
        return _state.to<Type>( _pos );
    }

    bool is_top() const {
        return _state.get_top() == _pos;
    }

    template<typename Type>
    stack_slot<State>& operator=( Type&& value_ ) { set( std::forward<Type>( value_ ) ); return *this; }

    template<typename Type>
    operator Type() const { return to<Type>(); }

    template<typename... Args>
    void call( Args&&... args_ ) {
        _state.push( std::forward<Args>( args_ )... );
        auto top = _state.get_top();
        _state.call( top - _pos, LUA_MULTRET );
    }

    template<typename... Args>
    int pcall( int error_handler_, Args&&... args_ ) {
        _state.push( std::forward<Args>( args_ )... );
        auto top = _state.get_top();
        return _state.pcall( top - _pos, LUA_MULTRET, error_handler_ );
    }

    template<typename State, typename... Args>
    int pcall( const stack_slot<State>& error_handler_, Args&&... args_ ) {
        assert( state() == error_handler_.state() );
        _state.push( std::forward<Args>( args_ )... );
        auto top = _state.get_top();
        return _state.pcall( top - _pos, LUA_MULTRET, error_handler_.index() );
    }

    shared_var<State> make_shared_var() {
        return shared_var<State> { _state, _pos };
    }
};

template<typename Type>
inline bool operator ==( const stack_slot<Type>& lhv_, stack_slot<Type>& rhv_ ) {
    assert( lhv_ == rhv_ );
    return lhv_.state().equal( lhv_.index(), rhv_.index() );
}
}