#pragma once

#include <utility>
#include <cassert>
#include <memory>

#include "lua_type_traits.h"
#include "lua_light_user_data.h"
#include "lua_push.h"
#include "lua_to.h"

namespace lua {
template<typename State, typename TableRef, typename IndexRef>
class var {
    mutable State       _state;
    mutable TableRef    _table;
    mutable IndexRef    _index;

public:
    var( State state_, TableRef table_, IndexRef index_ ) : _state { state_ }, _table { table_ }, _index { index_ } {}
    var( State state_, TableRef table_, IndexRef index_, int init_from_, bool remove_ ) : _state { state_ }, _table { table_ }, _index { index_ } {
        _table.set_at_from_stack( _state, _index, init_from_ );
        if ( remove_ ) {
            _state.remove( init_from_ );
        }
    }
    ~var() = default;

    State& state() const {
        return _state;
    }

    TableRef& table() const {
        return _table;
    }

    IndexRef& index() const {
        return _index;
    }

    template<typename Type>
    void set( Type value_ ) {
        _table.set_at( _state, _index, std::forward<Type>( value_ ) );
    }

    template<typename Type>
    Type get() const {
        return _table.get_at<Type>( _state, _index );
    }

    template<typename Type>
    var& operator=( Type value_ ) {
        set( std::forward<Type>( value_ ) );
        return *this;
    }
    
    template<typename Type>
    operator Type() const {
        return get<Type>();
    }

    int push() const {
        return _table.push_at( _state, _index );
    }
};

template<typename TableRef, typename IndexRef>
struct var_on_stack_as_table {
    TableRef    _table;
    IndexRef    _index;
    template<typename State, typename Index>
    void set_at_from_stack( State& state_, Index& index_, int init_from_ ) {
        int at = _table.push_at( state_, _index );
        index_.push( state_ );
        state_.push_value( init_from_ );
        state_.set_table( at );
        state_.remove( at );
    }

    template<typename State, typename Index, typename Type>
    void set_at( State& state_, Index& index_, Type value_ ) {
        int at = _table.push_at( state_, _index );
        index_.push( state_ );
        state_.push( std::forward<Value>( value_ ) );
        state_.set_table( at );
        state_.remove( at );
    }

    template<typename Type, typename State, typename Index>
    Type get_at( State& state_, Index& index_ ) {
        return state_.to<Type>( push_at( state_, index_ ) );
    }

    template<typename State, typename Index>
    int push_at( State& state_, Index& index_ ) {
        int at = _table.push_at( state_, _index );
        index_.push( state_ );
        state_.get_table( at );
        state_.remove( at );
        return state_.get_top();
    }
};

template<int IndexValue>
struct raw_static_index_table {
    template<typename State, typename Index>
    void set_at_from_stack( State& state_, Index& index_, int init_from_ ) {
        index_.push( state_ );
        state_.push_value( init_from_ );
        state_.raw_set( IndexValue );
    }

    template<typename State, typename Index, typename Type>
    void set_at( State& state_, Index& index_, Type value_ ) {
        index_.push( state_ );
        state_.push( std::forward<Value>( value_ ) );
        state_.raw_set( IndexValue );
    }

    template<typename Type, typename State, typename Index>
    Type get_at( State& state_, Index& index_ ) {
        return state_.to<Type>( push_at( state_, index_ ) );
    }

    template<typename State, typename Index>
    int push_at( State& state_, Index& index_ ) {
        index_.push( state_ );
        state_.raw_get( IndexValue );
        return state_.get_top();
    }
};

struct stack_index {
    int _value;
};

template<int Value>
struct constant_index {
    enum { _value = Value };
};

struct stack_index_table {
    template<typename State>
    void set_at_from_stack( State& state_, stack_index index_, int init_from_ ) {
        state_.push_value( init_from_ );
        state_.replace( index_._value );
    }

    template<typename State, typename Type>
    void set_at( State& state_, stack_index index_, Type value_ ) {
        state_.push( std::forward<Type>( value_ ) );
        state_.replace( index_._value );
    }

    template<typename Type, typename State>
    Type get_at( State& state_, stack_index index_ ) {
        return state_.to<Type>( index_._value );
    }

    template<typename State>
    int push_at( State& state_, stack_index index_ ) {
        state_.push_value( index_._value );
        return state_.get_top();
    }
};

struct self_pointer_index {
    template<typename State>
    void push( State& state_ ) {
        state_.push( light_user_data { this } );
    }
    void push( ::lua_State* state_ ) {
        ::lua::push( state_, light_user_data { this } );
    }
};

struct value_at_stack_as_index {
    int _pos;
    template<typename State>
    void push( State& state_ ) {
        state_.push_value( _pos );
    }
    void push( ::lua_State* state_ ) {
        lua_pushvalue( state_, _pos );
    }
};

template<typename TableRef, typename IndexRef>
struct var_as_index {
    TableRef    _table;
    IndexRef    _index;

    template<typename State>
    void push( State& state_ ) {
        _table.push_at( state_, _index );
    }
    void push( ::lua_State* state_ ) {
        _table.push_at( context { state_ }, _index );
    }
};

class any_type_as_index {
    struct concept {
        virtual ~concept() = default;
        virtual concept* copy() const = 0;
        virtual void push( ::lua_State* ) = 0;
        virtual int value() = 0;
    };

    template<typename Type>
    struct model {
        Type    _value;
        template<typename Init>
        model( Init init_ ) : _value { std::forward<Init>( init_ ) } {}
        virtual concept* copy() const override { return new model { _value }; }
        virtual void push( ::lua_State* state_ ) override {
            _value.push( state_ );
        }
        virtual int value() {
            return _value.value();
        }
    };

    std::unique_ptr<concept>    _inst;

public:
    any_type_as_index() = default;
    ~any_type_as_index() = default;

    any_type_as_index( const any_type_as_index& other_ )
        : _inst { other_._inst ? other_._inst->copy() : nullptr } {}

    any_type_as_index& operator=( const any_type_as_index& other_ ) {
        _inst.reset( other_._inst ? other_._inst->copy() : nullptr );
        return *this;
    }

    any_type_as_index( any_type_as_index && ) = default;
    any_type_as_index& operator=( any_type_as_index&& ) = default;

    template<typename Type>
    any_type_as_index( Type init_ ) : _inst { std::make_unique<model<std::decay_t<Type>>>( std::forward<Type>( init_ ) ); }
    template<typename State>
    void push( State& state_ ) {
        _inst->push( state_.get() );
    }
    int value() {
        return _inst->value();
    }
};



template<typename State, typename TableRef, typename IndexRef>
inline var<std::decay_t<State>, raw_static_index_table<LUA_REGISTRYINDEX>, self_pointer_index> make_shared( const var<State, TableRef, IndexRef>& var_ ) {
    return var<std::decay_t<State>, raw_static_index_table<LUA_REGISTRYINDEX>, self_pointer_index> { var_.state(), {}, {}, var_.push(), true };
}

template<typename State, typename TableRef, typename IndexRef, typename TableRef2, typename IndexRef2, typename State2>
inline var<State, var_on_stack_as_table< TableRef, IndexRef>, var_as_index<TableRef2, IndexRef2>> index_table( const var<State, TableRef, IndexRef>& table_, const var<State2, TableRef2, IndexRef2>& index_ ) {
    return var<State, var_on_stack_as_table<TableRef, IndexRef>, var_as_index<TableRef2, IndexRef2>> {table_.state(), { table_.table(), table_.index() }, { index_.table(), index_.index() }};
}
}