#include "../include/luawrapp.h"

#include <utility>

void test() {
    lua::context ctx {};

    ctx.open( {} );

    auto cpy = std::move(ctx);

    lua::shared_context ctx2 {};
    ctx2.open( {} );

    auto copy = ctx2;

    unsigned m = 1 << lua::context::debug_event::call;

    auto mask = lua::make_debug_mask( lua::context::debug_event::call, lua::context::debug_event::line, lua::context::debug_event::ret, lua::context::debug_event::tail_ret );
    copy.set_hook( nullptr, mask, 0 );

    lua::push( copy.get(), 1 );
    lua::push( copy.get(), true );
    lua::push( copy.get(), 1.f );
    lua::push( copy.get(), 1.0 );
    lua::push( copy.get(), nullptr );
    lua::push( copy.get(), lua::nil {} );
    lua::push( copy.get(), copy );
}

void main() {
    test();
}