#pragma once

namespace lua {
template<typename Context>
class stack_value {
    Context _ctx;
    int     _pos;

public:
    stack_value() = delete;
    stack_value( Context ctx_, int pos_ ) : _ctx { std::forward<Context>( ctx_ ) }, _pos { pos_ } {}
    ~stack_value() = default;

    stack_value( const stack_value& ) = default;
    stack_value& operator=( const stack_value& ) = default;

    stack_value( stack_value&& ) = default;
    stack_value& operator=( stack_value&& ) = default;

    template<typename Type>
    bool is() const {
        return _ctx.is<Type>( _pos );
    }

    template<typename Type>
    Type to() const {
        return _ctx.to<Type>( _pos );
    }
};
}