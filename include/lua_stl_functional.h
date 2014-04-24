#pragma once

#include <functional>

namespace lua {
template<typename Ret, typename... Args>
struct type_trait<std::function<Ret(Args...)>> {
    typedef std::function<Ret( Args... )> value_type;
    static const char* type_name() {
        // '\0' + len of "std::function[]" + two chars per byte
        static char name[1 + 15 + sizeof(void*)* 2] = { 0 };
        if ( !name[0] ) {
            // using pointer to this function as a uniqe id
            sprintf_s( name, "std::function[%p]", &type_name );
        }
        return name;
    }

    static bool is( ::lua_State* l_, int index_ ) {
        return luaL_checkudata( l_, index_, type_name() );
    }

    static value_type& to( ::lua_State* l_, int index_ ) {
        auto self = static_cast<value_type*>( luaL_checkudata( l_, index_, type_name() ) );
        if ( !self ) {
            luaL_typerror( l_, index_, type_name() );
            throw std::runtime_error { "bad lua var" };
        }
        return *self;
    }

    static bool validate_params( ::lua_State* l_ ) {
        if ( !is( l_, 1 ) ) {
            return false;
        }
        return ::lua::is<Args...>( l_, 2, 1 );
    }

    template<int... Seq>
    static void invoke( ::lua_State* l_, value_type& functor_, tools::seq<Seq...> ) {
        ::lua::push( l_, functor_( ::lua::to<Args>( l_, 2 + Seq )... ) );
    }

    static void register_this( ::lua_State* l_ ) {
        if ( !luaL_newmetatable( l_, type_name() ) ) {
            return;
        }

        lua_pushliteral( l_, "__gc" );
        lua_pushcclosure( l_, []( ::lua_State* l_ )->int {
            decltype( auto ) self = to( l_, -1 );
            self.~value_type();
            return 0;
        }, 0 );
        lua_settable( l_, -3 );

        lua_pushliteral( l_, "__call" );
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_params( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = to( l_, 1 );
            return invoke( l_, self, typename tools::gen_seq<sizeof...( Args )>::type {} );
        }, 0 );
        lua_settable( l_, -3 );
    }

    static int push( ::lua_State* l_, value_type func_ ) {
        register_this( l_ );

        new ( lua_newuserdata( l_, sizeof( value_type ) ) ) value_type { std::move( func_ ) };
        luaL_getmetatable( l_, type_name() );
        lua_setmetatable( l_, -2 );
        return 1;
    }
};

template<typename... Args>
struct type_trait<std::function<void( Args... )>> {
    typedef std::function<void( Args... )> value_type;
    static const char* type_name() {
        // '\0' + len of "std::function[]" + two chars per byte
        static char name[1 + 15 + sizeof(void*) * 2] = { 0 };
        if ( !name[0] ) {
            // using pointer to this function as a uniqe id
            sprintf_s( name, "std::function[%p]", &type_name );
        }
        return name;
    }

    static bool is( ::lua_State* l_, int index_ ) {
        return nullptr != luaL_checkudata( l_, index_, type_name() );
    }

    static value_type& to( ::lua_State* l_, int index_ ) {
        auto self = static_cast<value_type*>( luaL_checkudata( l_, index_, type_name() ) );
        if ( !self ) {
            luaL_typerror( l_, index_, type_name() );
            throw std::runtime_error { "bad lua var" };
        }
        return *self;
    }

    static bool validate_params( ::lua_State* l_ ) {
        if ( !is( l_, 1 ) ) {
            return false;
        }
        return ::lua::is<Args...>( l_, 2, 1 );
    }

    template<int... Seq>
    static void invoke( ::lua_State* l_, value_type& functor_, tools::seq<Seq...> ) {
        functor_( ::lua::to<Args>( l_, 2 + Seq )... );
    }

    static void register_this( ::lua_State* l_ ) {
        if ( !luaL_newmetatable( l_, type_name() ) ) {
            return;
        }

        lua_pushliteral( l_, "__gc" );
        lua_pushcclosure( l_, []( ::lua_State* l_ )->int {
            decltype( auto ) self = to( l_, -1 );
            self.~value_type();
            return 0;
        }, 0 );
        lua_settable( l_, -3 );

        lua_pushliteral( l_, "__call" );
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_params( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = to( l_, 1 );
            invoke( l_, self, typename tools::gen_seq<sizeof...( Args )>::type {} );
            return 0;
        }, 0 );
        lua_settable( l_, -3 );
    }

    static int push( ::lua_State* l_, value_type func_ ) {
        register_this( l_ );

        new ( lua_newuserdata( l_, sizeof( value_type ) ) ) value_type { std::move( func_ ) };
        luaL_getmetatable( l_, type_name() );
        lua_setmetatable( l_, -2 );
        return 1;
    }
};

template<typename Ret>
struct type_trait<std::function<Ret()>> {
    typedef std::function<Ret()> value_type;
    static const char* type_name() {
        // '\0' + len of "std::function[]" + two chars per byte
        static char name[1 + 15 + sizeof(void*)* 2] = { 0 };
        if ( !name[0] ) {
            // using pointer to this function as a uniqe id
            sprintf_s( name, "std::function[%p]", &type_name );
        }
        return name;
    }

    static bool is( ::lua_State* l_, int index_ ) {
        return luaL_checkudata( l_, index_, type_name() );
    }

    static value_type& to( ::lua_State* l_, int index_ ) {
        auto self = static_cast<value_type*>( luaL_checkudata( l_, index_, type_name() ) );
        if ( !self ) {
            luaL_typerror( l_, index_, type_name() );
            throw std::runtime_error { "bad lua var" };
        }
        return *self;
    }

    static bool validate_params( ::lua_State* l_ ) {
        return is( l_, 1 );
    }

    static void register_this( ::lua_State* l_ ) {
        if ( !luaL_newmetatable( l_, type_name() ) ) {
            return;
        }

        lua_pushliteral( l_, "__gc" );
        lua_pushcclosure( l_, []( ::lua_State* l_ )->int {
            decltype( auto ) self = to( l_, -1 );
            self.~value_type();
            return 0;
        }, 0 );
        lua_settable( l_, -3 );

        lua_pushliteral( l_, "__call" );
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_params( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = to( l_, 1 );
            return ::lua::push( l_, self() );
        }, 0 );
        lua_settable( l_, -3 );
    }

    static int push( ::lua_State* l_, value_type func_ ) {
        register_this( l_ );

        new ( lua_newuserdata( l_, sizeof( value_type ) ) ) value_type { std::move( func_ ) };
        luaL_getmetatable( l_, type_name() );
        lua_setmetatable( l_, -2 );
        return 1;
    }
};

template<>
struct type_trait<std::function<void()>> {
    typedef std::function<void()> value_type;
    static const char* type_name() {
        // '\0' + len of "std::function[]" + two chars per byte
        static char name[1 + 15 + sizeof(void*)* 2] = { 0 };
        if ( !name[0] ) {
            // using pointer to this function as a uniqe id
            sprintf_s( name, "std::function[%p]", &type_name );
        }
        return name;
    }

    static bool is( ::lua_State* l_, int index_ ) {
        return luaL_checkudata( l_, index_, type_name() );
    }

    static value_type& to( ::lua_State* l_, int index_ ) {
        auto self = static_cast<value_type*>( luaL_checkudata( l_, index_, type_name() ) );
        if ( !self ) {
            luaL_typerror( l_, index_, type_name() );
            throw std::runtime_error { "bad lua var" };
        }
        return *self;
    }

    static bool validate_params( ::lua_State* l_ ) {
        return is( l_, 1 );
    }

    static void register_this( ::lua_State* l_ ) {
        if ( !luaL_newmetatable( l_, type_name() ) ) {
            return;
        }

        lua_pushliteral( l_, "__gc" );
        lua_pushcclosure( l_, []( ::lua_State* l_ )->int {
            decltype( auto ) self = to( l_, -1 );
            self.~value_type();
            return 0;
        }, 0 );
        lua_settable( l_, -3 );

        lua_pushliteral( l_, "__call" );
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_params( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = to( l_, 1 );
            self();
            return 0;
        }, 0 );
        lua_settable( l_, -3 );
    }

    static int push( ::lua_State* l_, value_type func_ ) {
        register_this( l_ );

        new ( lua_newuserdata( l_, sizeof( value_type ) ) ) value_type { std::move( func_ ) };
        luaL_getmetatable( l_, type_name() );
        lua_setmetatable( l_, -2 );
        return 1;
    }
};
}