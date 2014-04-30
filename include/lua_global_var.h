#pragma once

namespace lua {
template<typename Context>
class global_var {
    shared_var<Context> _key;

public:
    global_var() = default;
    ~global_var() = default;

    global_var( const global_var& ) = default;
    global_var& operator=( const global_var& ) = default;

    global_var( global_var&& ) = default;
    global_var& operator=( global_var&& ) = default;

    Context& context() { return _key.context(); }

    template<typename KeyType>
    global_var( Context ctx_, KeyType key_ ) : _key { std::forward<Context>( ctx_ ), ( ctx_.push( std::forward<KeyType>( key_ ) ), ctx_.get_top() ) } {}

    int push() {
        _key.push();
        _key.context().get_table( LUA_GLOBALSINDEX );
        return _key.context().get_top();
    }

    template<typename Type>
    void set( Type value_ ) {
        _key.push();
        _key.context().push( std::forward<Type>( value_ ) );
        _key.context().set_table( LUA_GLOBALSINDEX );
    }

    void replace( int index_ ) {
        _key.push();
        _key.context().push_value( index_ );
        _key.context().set_table( LUA_GLOBALSINDEX );
        _key.context().remove( index_ );
    }
};
}