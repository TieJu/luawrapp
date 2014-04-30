#pragma once

namespace lua {
template<typename Context>
class stack_var {
    Context _ctx;
    int     _id { 0 };

public:
    stack_var() = default;
    ~stack_var() = default;

    stack_var( const stack_var & ) = default;
    stack_var& operator=( const stack_var & ) = default;

    stack_var( stack_var&& ) = default;
    stack_var& operator=( stack_var&& ) = default;

    stack_var( Context ctx_, int id_ ) : _ctx { ctx_ }, _id { id_ } {}

    int push() {
        _ctx.push_value( _id );
        return _ctx.get_top();
    }

    template<typename Type>
    bool is() {
        return _ctx.is<Type>( _id );
    }

    template<typename Type>
    auto to() -> decltype( _ctx.to<Type>( 0 ) ) {
        return _ctx.to<Type>( _id );
    }

    template<typename Type>
    void set( Type value_ ) {
        _ctx.push( std::forward<Type>( value_ ) );
        _ctx.replace( _id );
    }

    void replace( int index_ ) {
        if ( index_ != -1 && index_ != _ctx.get_top() ) {
            _ctx.push_value( index_ );
            _ctx.remove( index_ );
        } else {
            _ctx.replace( _id );
        }
    }

    void replace( stack_var& other_ ) {
        other_.push();
        _ctx.replace( _id );
    }
};
}