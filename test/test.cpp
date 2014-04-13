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

    copy.push( 1 );
    copy.push( true );
    copy.push( 1.f );
    copy.push( 1.0 );
    copy.push( nullptr );
    copy.push( lua::nil {} );

    copy.pop( 2 );

    copy.to<double>( -1 );
    copy.to<float>( -2 );
    copy.to<bool>( -3 );
    copy.to<int>( -4 );

    copy.push( 1, 2, 3.f, 4.7, 5, 6, 7, 8, 9, true );
}

void main() {
    test();
}