#pragma once
namespace lua {
template<typename Class, typename Derived>
struct class_trait_base {
    static std::tuple<int, int> begin_class_reg( ::lua_State* l_ ) {
        lua_newtable( l_ );
        int methods = lua_gettop( l_ );

        luaL_newmetatable( l_, Derived::name );
        int metatable = lua_gettop( l_ );

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushstring( l_, Derived::name );
        lua_pushvalue( l_, methods );
        lua_settable( l_, LUA_GLOBALSINDEX );

        lua_pushliteral( l_, "__metatable" );
        lua_pushvalue( l_, methods );
        lua_settable( l_, metatable );  // hide metatable from Lua getmetatable()

        lua_pushliteral( l_, "__index" );
        lua_pushvalue( l_, methods );
        lua_settable( l_, metatable );

        lua_pushliteral( l_, "__tostring" );
        lua_pushcfunction( l_, Derived::on_to_string );
        lua_settable( l_, metatable );

        lua_pushliteral( l_, "__gc" );
        lua_pushcfunction( l_, Derived::on_gc );
        lua_settable( l_, metatable );

        lua_newtable( l_ );                // mt for method table
        int mt = lua_gettop( l_ );
        lua_pushliteral( l_, "__call" );
        lua_pushcfunction( l_, Derived::on_new );
        lua_pushliteral( l_, "new" );
        lua_pushvalue( l_, -2 );           // dup new_T function
        lua_settable( l_, methods );       // add new_T to method table
        lua_settable( l_, mt );            // mt.__call = new_T
        lua_setmetatable( l_, methods );

        return std::make_tuple( metatable, methods );
    }

    static void end_class_reg( ::lua_State* l_ ) {
        lua_pop( l_, 2 );
    }

    template<typename RetType, typename... Args, int... Seq>
    static int invoke_function( ::lua_State* l_, RetType( *fp_ )( Args... ), int first_, int step_, tools::seq<Seq...> ) {
        return push( l_, fp_( to<Args>( l_, first_ + Seq * step_ )... ) );
    }

    template<typename... Args, int... Seq>
    static void invoke_function_nr( ::lua_State* l_, void( *fp_ )( Args... ), int first_, int step_, tools::seq<Seq...> ) {
        fp_( to<Args>( l_, first_ + Seq * step_ )... );
    }

    template<typename RetType, typename... Args, int... Seq>
    static int inoke_method( ::lua_State* l_, Class* self_, RetType( Class::*mp_ )( Args... ), int first_, int step_, tools::seq<Seq...> ) {
        return push( l_, (self_->*mp_)( to<Args>( l_, first_ + Seq * step_ )... ) );
    }

    template<typename... Args, int... Seq>
    static void inoke_method_nr( ::lua_State* l_, Class* self_, void( Class::*mp_ )( Args... ), int first_, int step_, tools::seq<Seq...> ) {
        ( self_->*mp_ )( to<Args>( l_, first_ + Seq * step_ )... );
    }

    template<typename RetType, typename... Args>
    static int function_proxy( ::lua_State *l_ ) {
        typedef RetType( *callback )( );
        if ( !validate_function_params<Args...>( l_ ) ) {
            return 0;
        }
        auto func = *static_cast<const callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        return invoke_function( l_, func, 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
    }

    template<typename... Args>
    static int function_proxy_nr( ::lua_State *l_ ) {
        typedef void( *callback )( );
        if ( !validate_function_params<Args...>( l_ ) ) {
            return 0;
        }
        auto func = *static_cast<const callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        invoke_function_nr( l_, func, 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        return 0;
    }

    template<typename RetType>
    static int function_proxy_np( ::lua_State *l_ ) {
        typedef RetType( *callback )( );
        if ( !validate_function_params_np( l_ ) ) {
            return 0;
        }
        auto func = *static_cast<const callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        return push( l_, func() );
    }

    static int function_proxy_nr_np( ::lua_State *l_ ) {
        typedef void( *callback )( );
        if ( !validate_function_params_np( l_ ) ) {
            return 0;
        }
        auto func = *static_cast<const callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        func();
        return 0;
    }

    template<typename RetType, typename... Args>
    static int method_proxy( ::lua_State *l_ ) {
        typedef RetType( Class::*member_callback )( Args... args_ );
        if ( !validate_method_params<Args...>( l_ ) ) {
            return 0;
        }
        auto& self = Derived::to( l_, 1 );
        auto member = *static_cast<const member_callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        return inoke_method( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
    }

    template<typename... Args>
    static int method_proxy_nr( ::lua_State *l_ ) {
        typedef void( Class::*member_callback )( Args... args_ );
        if ( !validate_method_params<Args...>( l_ ) ) {
            return 0;
        }
        auto& self = Derived::to( l_, 1 );
        auto member = *static_cast<const member_callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        inoke_method_nr( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        return 0;
    }

    template<typename RetType>
    static int method_proxy_np( ::lua_State *l_ ) {
        typedef RetType( Class::*member_callback )( );
        if ( !validate_method_params_np( l_ ) ) {
            return 0;
        }
        auto& self = Derived::to( l_, 1 );
        auto member = *static_cast<const member_callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        push( l_, ( self.*member )( ) );
        return 1;
    }

    static int method_proxy_nr_np( ::lua_State *l_ ) {
        typedef void( Class::*member_callback )( );
        if ( !validate_method_params_np( l_ ) ) {
            return 0;
        }
        auto& self = Derived::to( l_, 1 );
        auto member = *static_cast<const member_callback*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        ( self.*member )( );
        return 0;
    }

    template<typename... Args>
    static bool validate_function_params( ::lua_State* l_ ) {
        if ( lua_gettop( l_ ) != sizeof...( Args ) ) {
            return false;
        }
        return is<Args...>( l_, 1, 1 );
    }

    static bool validate_function_params_np( ::lua_State*l_ ) {
        return lua_gettop( l_ ) == 0;
    }

    template<typename... Args>
    static bool validate_method_params( ::lua_State* l_ ) {
        if ( lua_gettop( l_ ) != sizeof...( Args ) + 1 ) {
            return false;
        }
        if ( !Derived::is( l_, 1 ) ) {
            return false;
        }
        return is<Args...>( l_, 2, 1 );
    }

    static bool validate_method_params_np( ::lua_State*l_ ) {
        if ( lua_gettop( l_ ) != 1 ) {
            return false;
        }
        return Derived::is( l_, -1 );
    }

    static void begin_overloaded_method( ::lua_State* l_, int medthod_table_, int metha_table_, const char* name_ ) {}
    template<typename RetType, typename... Args>
    static void add_overloaded_method_variant( ::lua_State* l_, int medthod_table_, int metha_table_, RetType( Class::*method_ )( Args... ) ) {}
    template<typename... Args>
    static void add_overloaded_method_variant( ::lua_State* l_, int medthod_table_, int metha_table_, void( Class::*method_ )( Args... ) ) {}
    template<typename RetType>
    static void add_overloaded_method_variant( ::lua_State* l_, int medthod_table_, int metha_table_, RetType( Class::*method_ )( ) ) {}
    static void add_overloaded_method_variant( ::lua_State* l_, int medthod_table_, int metha_table_, void( Class::*method_ )( ) ) {}
    static void end_overloaded_method( ::lua_State* l_, int medthod_table_, int metha_table_ ) {}

    template<typename RetType, typename... Args>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, RetType( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, &method_proxy<RetType, Args...>, 1 );
        lua_settable( l_, medthod_table_ );
    }
    template<typename... Args>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, void( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, &method_proxy_nr<Args...>, 1 );
        lua_settable( l_, medthod_table_ );
    }
    template<typename RetType>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, RetType( Class::*method_ )( ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, &method_proxy_np<RetType>, 1 );
        lua_settable( l_, medthod_table_ );
    }
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, void( Class::*method_ )( ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, &method_proxy_nr_np, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void begin_overloaded_function( ::lua_State* l_, int medthod_table_, int metha_table_, const char* name_ ) {}
    template<typename RetType, typename... Args>
    static void add_overloaded_function_variant( ::lua_State* l_, int medthod_table_, int metha_table_, RetType( *function_ )( Args... ) ) {}
    template<typename... Args>
    static void add_overloaded_function_variant( ::lua_State* l_, int medthod_table_, int metha_table_, void( *function_ )( Args... ) ) {}
    template<typename RetType>
    static void add_overloaded_function_variant( ::lua_State* l_, int medthod_table_, int metha_table_, RetType( *function_ )( ) ) {}
    static void add_overloaded_function_variant( ::lua_State* l_, int medthod_table_, int metha_table_, void( *function_ )( ) ) {}
    static void end_overloaded_function( ::lua_State* l_, int medthod_table_, int metha_table_ ) {}

    template<typename RetType, typename... Args>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, RetType( *function_ )( Args... ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, &function_proxy<RetType, Args...>, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename... Args>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, void( *function_ )( Args... ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, &function_proxy_nr<Args...>, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename RetType>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, RetType( *function_ )( ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, &function_proxy_np<RetType>, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, void( *function_ )( ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, &function_proxy_nr_np, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void add_raw_function( ::lua_State* l_, const char* name_, int medthod_table_, int metha_table_, lua_CFunction func_ ) {
        lua_pushstring( l_, name_ );
        lua_pushcclosure( l_, func_, 0 );
        lua_settable( l_, medthod_table_ );
    }

    static int on_to_string( ::lua_State* l_ ) {
        auto& self = Derived::to( l_, 1 );
        lua_pushfstring( l_, "%s (%08p)", Derived::name, &self );
        return 1;
    }

    static int on_gc( ::lua_State* l_ ) {
        auto& self = Derived::to( l_, 1 );
        self.~Class();
        return 0;
    }

    static int on_new( ::lua_State* l_ ) {
        lua_getglobal( l_, Derived::name );
        if ( lua_equal( l_, -1, 1 ) ) {
            lua_remove( l_, 1 );    // first param is table if the __call is called
        }
        lua_pop( l_, 1 );
        auto inst = Derived::create( l_ );

        if ( inst ) {
            luaL_getmetatable( l_, Derived::name );
            lua_setmetatable( l_, -2 );
            return 1;
        } else {
            luaL_error( l_, "no matching constructor for %s found", Derived::name );
            return 1;
        }
    }
};
}