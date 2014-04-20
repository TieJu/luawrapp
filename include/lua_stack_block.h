#pragma once

namespace lua {
template<typename State>
class stack_block
    : public stack<stack_block<State>>{
    State   _state;
    int     _top;

public:
    stack_block( State state_ ) : _state { std::move( state_ ) } { recapture_top(); }
    ~stack_block() { reset_top(); }

    stack_block( const stack_block & ) = delete;
    stack_block& operator=( const stack_block & ) = delete;

    stack_block( stack_block&& other_ ) { *this = std::move( other_ ); }
    stack_block& operator=( stack_block&& other_ ) {
        if ( _state ) {
            reset_top();
        }
        _state = std::move( other_._state );
        std::swap( _top, other_._top );
        return *this;
    }

    auto get()->decltype( _state.get() ) { return _state.get(); }

    void recapture_top() {
        _top = _state.get_top();
    }

    void reset_top() {
        if ( _top >= 0 ) {
            _state.set_top( _top );
            _top = -1;
        }
    }
};
}