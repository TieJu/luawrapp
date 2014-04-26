#pragma once

namespace lua {
template<typename Context>
class stack : public stack_base<stack<Context>> {
    Context _ctx;
    int     _base;

public:
    stack( Context ctx_ ) : _ctx { std::forward<Context>( ctx_ ) } { _base = _ctx.get_top(); }
    ~stack() { _ctx.set_top( _base ); }

    auto get() const -> decltype(_ctx.get()) { return _ctx.get(); }
    auto get() -> decltype( _ctx.get() ) { return _ctx.get(); }

    void reserve( size_t size_ ) { _ctx.check_stack( _base + size_ ); }
    void resize( size_t size_ ) { _ctx.set_top( _base + size_ ); }

    stack_iterator<Context> begin() { return { _ctx, _base ? _base : _ctx.get_top() ? 1 : 0 }; }
    stack_iterator<Context> end() { return { _ctx, _ctx.get_top() }; }
};
}