#include "../include/luawrapp.h"

#include <utility>

#include "../include/lua_type_traits.h"

template<typename Ret,typename Func,typename Tup,int... Seq>
inline Ret invoke( Func func_, Tup&& tup_, lua::tools::seq<Seq...> ) {
    return func_( std::get<Seq>( tup_ )... );
}

template<typename RetType, typename... InArgs>
inline int call_proxy( ::lua_State* l_ ) {
    typedef RetType( *func )( InArgs...args_ );
    auto func_ = static_cast<func>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
    auto params = lua::to<InArgs...>( l_, 1, 1 );
    lua::push( l_, invoke<RetType>( func_, params, typename lua::tools::gen_seq<sizeof...( InArgs )>::type {} ) );
    return 1;
}

template<typename RetType, typename... InArgs>
inline void wrap_func( ::lua_State*l_, RetType( *func )( InArgs... ) ) {
    auto proxy = &call_proxy<RetType, InArgs...>;
    lua_pushlightuserdata( l_, func );
    lua_pushcclosure( l_, proxy, 1 );
}

int test_func( int, float ) {
    return 0;
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
    copy.push( std::make_tuple( 1, 2.f, true ) );

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

    wrap_func( copy.get(), &test_func );
}

void main() {
    test();
}