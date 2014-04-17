#include "../include/luawrapp.h"

#include <utility>

#include "../include/lua_type_traits.h"

int test_func( int, float ) {
    return 0;
}

int test_func2( ::lua_State* l_, int, float ) {
    return 0;
}

void test() {
    lua::context ctx {};

    ctx.open( {} );

    auto cpy = std::move(ctx);

    auto c_var = cpy.push_to_stack( 5 );
    auto scvar = make_shared( c_var );

    //lua::unique_context uctx {};
    //uctx.open( {} );

    //auto u_var = uctx.push_to_stack( 7 );
    //auto suvar = make_shared( u_var );

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
    copy.push( lua::nil {} );
    copy.push( std::make_tuple( 1, 2.f, true ) );
    copy.push( test_func );
    copy.push( test_func2 );

    copy.pop( 1 );

    copy.to<double>( -1 );
    copy.to<float>( -2 );
    copy.to<bool>( -3 );
    copy.to<int>( -4 );

    copy.push( 1, 2, 3.f, 4.7, 5, 6, 7, 8, 9, true );
    auto set = copy.to<int, int, float, bool>( -1, -1 );

    auto res = lua::call<int, float, bool>( copy.get(), 1, 2.f, true );

    auto str = copy.to<const char*>( -1 );
    auto str2 = copy.to<std::string>( -2 );

    auto var = copy.get_at_stack( 5 );
    auto svar = make_shared( var );
    auto gvar = copy.get_global_var( "test" );
    auto ivar = index_table( var, svar );
    auto sivar = make_shared( ivar );
}

void main() {
    test();
}