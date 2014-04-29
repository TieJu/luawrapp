#pragma once

namespace lua {
template<typename Context>
class table_iterator {
    Context     _ctx;

    light_user_data table_id() const { return { this }; }
    light_user_data index_id() const { return { reinterpret_cast<const char*>( this ) + 1 }; }
    light_user_data value_id() const { return { reinterpret_cast<const char*>( this ) + 2 }; }

    bool next() {
        auto table = _ctx.get_table_entry( LUA_REGISTRYINDEX, table_id() );
        _ctx.get_table_entry( LUA_REGISTRYINDEX, index_id() );
        if ( !_ctx.next( table ) ) {
            _ctx.pop();
            clear();
            return false;
        }
        _ctx.push( value_id() );
        _ctx.insert( -2 );
        _ctx.set_table( LUA_REGISTRYINDEX );

        _ctx.push( index_id() );
        _ctx.insert( -2 );
        _ctx.set_table( LUA_REGISTRYINDEX );

        _ctx.pop( 1 );
        return true;
    }

    void clear() {
        _ctx.set_table_entry( LUA_REGISTRYINDEX, table_id(), nil {} );
        _ctx.set_table_entry( LUA_REGISTRYINDEX, index_id(), nil {} );
        _ctx.set_table_entry( LUA_REGISTRYINDEX, value_id(), nil {} );
    }

    void copy_from( const table_iterator& other_ ) {
        _ctx.push( table_id() );
        _ctx.get_table_entry( LUA_REGISTRYINDEX, other_.table_id() );
        _ctx.set_table( LUA_REGISTRYINDEX );

        _ctx.push( index_id() );
        _ctx.get_table_entry( LUA_REGISTRYINDEX, other_.index_id() );
        _ctx.set_table( LUA_REGISTRYINDEX );

        _ctx.push( value_id() );
        _ctx.get_table_entry( LUA_REGISTRYINDEX, other_.value_id() );
        _ctx.set_table( LUA_REGISTRYINDEX );
    }

public:
    table_iterator() = default;

    table_iterator( Context ctx_, int stack_pos_ ) : _ctx { std::forward<Context>( ctx_ ) } {
        _ctx.push( table_id() );
        _ctx.push_value( stack_pos_ );
        _ctx.set_table( LUA_REGISTRYINDEX );

        next();
    }

    ~table_iterator() {
        clear();
    }

    table_iterator( const table_iterator& other_ ) {
        copy_from( other_ );
    }
    table_iterator& operator=( const table_iterator& other_ ) {
        copy_from( other_ );
        return this;
    }

    table_iterator( table_iterator&& other_ ) {
        copy_from( other_ );
    }
    table_iterator& operator=( table_iterator&& other_ ) {
        copy_from( other_ );
        return this;
    }

    explicit operator bool() {
        return !empty();
    }

    bool empty() {
        _ctx.get_table_entry( LUA_REGISTRYINDEX, table_id() );
        bool is_empty = _ctx.is<nil>( -1 );
        _ctx.pop();
        return is_empty;
    }

    table_iterator& operator++( ) {
        next();
        return *this;
    }

    table_iterator operator++( int ) {
        auto copy = *this;
        next();
        return copy;
    }

    std::pair<shared_var<Context>, shared_var<Context>> operator*( ) {
        shared_var<Context> key { _ctx, _ctx.get_table_entry( LUA_REGISTRYINDEX, index_id() ) };
        shared_var<Context> value { _ctx, _ctx.get_table_entry( LUA_REGISTRYINDEX, value_id() ) };
        _ctx.pop( 2 );
        return { key, value };
    }
};
}