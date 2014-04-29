#pragma once

namespace lua {
template<typename Context>
class shared_var {
    Context     _ctx;

    light_user_data id() const { return { this }; }

    void clear() {
        _ctx.set_table_entry( LUA_REGISTRYINDEX, id(), nil {} );
    }

    void copy_from( const shared_var& other_ ) {
        _ctx.push( id() );
        _ctx.get_table_entry( LUA_REGISTRYINDEX, other_.id() );
        _ctx.set_table( LUA_REGISTRYINDEX );
    }

public:
    shared_var() = default;

    shared_var( Context ctx_, int stack_pos_ ) : _ctx { std::forward<Context>( ctx_ ) } {
        _ctx.push( id() );
        _ctx.push_value( stack_pos_ );
        _ctx.set_table( LUA_REGISTRYINDEX );
    }

    ~shared_var() {
        clear();
    }

    shared_var( const shared_var& other_ ) : _ctx { other_._ctx } {
        copy_from( other_ );
    }

    shared_var& operator=( const shared_var& other_ ) {
        _ctx = other_._ctx;
        copy_from( other_ );
        return this;
    }

    shared_var( shared_var&& other_ ) : _ctx { other_._ctx } {
        copy_from( other_ );
    }

    shared_var& operator=( shared_var&& other_ ) {
        _ctx = other_._ctx;
        copy_from( other_ );
        return this;
    }

    void push() {
        _ctx.get_table_entry( LUA_REGISTRYINDEX, id() );
    }

    template<typename Type>
    void set( Type value_ ) {
        _ctx.set_table_entry( LUA_REGISTRYINDEX, id(), std::forward<Type>( value_ ) );
    }
};
}