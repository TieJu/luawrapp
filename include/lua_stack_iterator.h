#pragma once

namespace lua {
template<typename Context>
class stack_iterator {
    Context _ctx;
    int     _pos;

public:
    stack_iterator() = delete;
    stack_iterator( Context ctx_, int pos_ ) : _ctx { std::forward<Context>( ctx_ ) }, _pos { pos_ } {}
    ~stack_iterator() = default;

    stack_iterator( const stack_iterator& ) = default;
    stack_iterator& operator=( const stack_iterator& ) = default;

    stack_iterator( stack_iterator&& ) = default;
    stack_iterator& operator=( stack_iterator&& ) = default;

    stack_iterator& operator++( ) {
        ++_pos;
        return *this;
    }
    stack_iterator operator++( int ) {
        auto tmp = *this;
        ++( *this );
        return tmp;
    }

    stack_iterator& operator--( ) {
        --_pos;
        return *this;
    }
    stack_iterator operator--( int ) {
        auto tmp = *this;
        --( *this );
        return tmp;
    }

    stack_value<Context> operator*( ) {
        return { _ctx, _pos };
    }

    inline bool operator==( const stack_iterator& rhv_ ) const {
        return _pos == rhv_._pos;
    }

    inline bool operator!=( const stack_iterator& rhv_ ) const {
        return _pos != rhv_._pos;
    }
};
}