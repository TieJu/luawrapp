#include "../include/luawrapp.h"

#include <utility>

#include "../include/lua_type_traits.h"

template<typename... RetArgs, typename... InArgs>
inline std::tuple<RetArgs...> call( lua_State* l_, InArgs... args_ ) {
    auto top = lua_gettop( l_ ) - 1;
    lua::push( l_, std::forward<InArgs>( args_ )... );
    lua_call( l_, sizeof...( InArgs ), sizeof...( RetArgs ) );
    return lua::to<RetArgs...>( l_, top, 1 );
}

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
    copy.push( lua::nil {} );

    copy.pop( 1 );

    copy.to<double>( -1 );
    copy.to<float>( -2 );
    copy.to<bool>( -3 );
    copy.to<int>( -4 );

    copy.push( 1, 2, 3.f, 4.7, 5, 6, 7, 8, 9, true );
    auto set = copy.to<int, int, float, bool>( -1, -1 );

    auto res = call<int, float, bool>( copy.get(), 1, 2.f, true );

    auto str = copy.to<const char*>( -1 );
    auto str2 = copy.to<std::string>( -2 );
}

void main() {
    test();
}