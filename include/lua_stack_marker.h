#pragma once

namespace lua {
template<typename State>
class stack_marker {
    State   _state;
    int     _pos;
public:
    stack_marker(State state_)
        : _state {state_}
        , _pos { state_.get_top() }
    {}

    ~stack_marker() {
        if ( _state.get() ) {
            _state.set_top( _pos );
        }
    }

    stack_marker( const stack_marker & ) = delete;
    stack_marker& operator=( const stack_marker & ) = delete;

    stack_marker( stack_marker&& ) = default;
    stack_marker& operator=( stack_marker&& ) = default;
};
}