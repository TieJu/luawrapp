#pragma once
namespace lua {
    inline void dump_stack_types( ::lua_State* l_ ) {
        for ( int i = 1; i <= lua_gettop( l_ ); ++i ) {
            printf( "[%i] %s\r\n", i, lua_typename( l_, lua_type( l_, i ) ) );
        }
    }
template<typename Class, typename Derived>
struct class_trait_base {
    struct class_reg_state {
        int     _meta_table
            ,   _method_table
            ,   _geter_table
            ,   _seter_table
            ,   _method_meta_table;
    };
    static class_reg_state begin_class_reg( ::lua_State* l_ ) {
        lua_newtable( l_ );
        int geter = lua_gettop( l_ );
        // local geter = {}

        lua_newtable( l_ );
        int seter = lua_gettop( l_ );
        // local seter = {}

        lua_newtable( l_ );
        int methods = lua_gettop( l_ );
        // local methods = {}

        luaL_newmetatable( l_, Derived::name );
        int metatable = lua_gettop( l_ );
        // local metatable = {}
        // local reg = debug.getregistry()
        // reg[<derived name>] = metatable

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushstring( l_, Derived::name );
        lua_pushvalue( l_, methods );
        lua_settable( l_, LUA_GLOBALSINDEX );
        // _G[<name>] = methods
        
        lua_pushliteral( l_, "__metatable" );
        lua_pushvalue( l_, methods );
        lua_settable( l_, metatable );  // hide metatable from Lua getmetatable()
        // metatable.__metatable = methods
        
        lua_pushliteral( l_, "__index" );
        lua_pushvalue( l_, methods );
        lua_settable( l_, metatable );
        // metatable.__index = methods
        
        lua_pushliteral( l_, "__tostring" );
        lua_pushcfunction( l_, Derived::on_to_string );
        lua_settable( l_, metatable );
        // metatable.__tostring = <derived to string>
        
        lua_pushliteral( l_, "__gc" );
        lua_pushcfunction( l_, Derived::on_gc );
        lua_settable( l_, metatable );
        // metatable.__gc = <derived gc>
        
        lua_newtable( l_ );                // mt for method table
        int mt = lua_gettop( l_ );
        lua_pushliteral( l_, "__call" );
        lua_pushcfunction( l_, Derived::on_new );
        lua_pushliteral( l_, "new" );
        lua_pushvalue( l_, -2 );           // dup new_T function
        lua_settable( l_, methods );       // add new_T to method table
        lua_settable( l_, mt );            // mt.__call = new_T
        lua_pushvalue( l_, mt );
        lua_setmetatable( l_, methods );
        // local mt = {}
        // mt._call = <derived new>
        // setmetatable(methods, mt)
        // methods.new = <derived new>

        return { metatable, methods, geter, seter, mt };
    }

    static void end_class_reg( ::lua_State* l_, class_reg_state tables_ ) {
        lua_pushnil( l_ );
        if ( lua_next( l_, tables_._geter_table ) ) {
            lua_pop( l_, 2 );
            lua_pushliteral( l_, "__index" );
            lua_pushvalue( l_, tables_._geter_table );
            lua_pushvalue( l_, tables_._method_table );
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                // table, key
                auto name = lua_tostring( l_, -1 );
                // table, key
                lua_pushvalue( l_, -1 );
                lua_gettable( l_, lua_upvalueindex( 2 ) );
                // table, key, func
                if ( !lua_isnil( l_, -1 ) ) {
                    return 1;
                } else {
                    // table, key
                    lua_pop( l_, 1 );
                }
                // table, clb
                lua_gettable( l_, lua_upvalueindex( 1 ) );
                if ( lua_isnil( l_, -1 ) ) {
                    luaL_error( l_, "%s is not a member of %s", name, Derived::name );
                    lua_pop( l_, 1 );
                    return 0;
                }
                // table, clb, table
                lua_pushvalue( l_, -2 );
                lua_call( l_, 1, 1 );
                return 1;
            }, 2 );
            lua_settable( l_, tables_._meta_table );
        } else {
            lua_pop( l_, 1 );
        }

        lua_pushnil( l_ );
        if ( lua_next( l_, tables_._seter_table ) ) {
            lua_pop( l_, 2 );
            lua_pushliteral( l_, "__newindex" );
            lua_pushvalue( l_, tables_._seter_table );
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                // table, key, value
                lua_pushvalue( l_, -2 );
                // table, key, value, key
                auto name = lua_tostring( l_, -1 );
                // table, value, key
                lua_remove( l_, -3 );
                // table, value, clb
                lua_gettable( l_, lua_upvalueindex( 1 ) );
                if ( lua_isnil( l_, -1 ) ) {
                    luaL_error( l_, "%s is not a member of %s", name, Derived::name );
                    lua_pop( l_, 1 );
                    return 0;
                }
                // table, value, clb, table
                lua_pushvalue( l_, -3 );
                // table, value, clb, table, value
                lua_pushvalue( l_, -3 );
                lua_call( l_, 2, 0 );
                return 0;
            }, 1 );
            lua_settable( l_, tables_._meta_table );
        } else {
            lua_pop( l_, 1 );
        }

        lua_pop( l_, 5 );
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

    template<typename... Args>
    static bool validate_function_params( ::lua_State* l_, int offset_ = 0 ) {
        if ( lua_gettop( l_ ) != sizeof...( Args ) + offset_ ) {
            return false;
        }
        return is<Args...>( l_, 1, 1 );
    }

    static bool validate_function_params_np( ::lua_State*l_, int offset_ = 0 ) {
        return lua_gettop( l_ ) == offset_;
    }

    template<typename... Args>
    static bool validate_method_params( ::lua_State* l_, int offset_ = 0 ) {
        if ( lua_gettop( l_ ) != sizeof...( Args ) + 1 + offset_) {
            return false;
        }
        if ( !Derived::is( l_, 1 ) ) {
            return false;
        }
        return is<Args...>( l_, 2, 1 );
    }

    static bool validate_method_params_np( ::lua_State*l_, int offset_ = 0 ) {
        if ( lua_gettop( l_ ) != 1 + offset_ ) {
            return false;
        }
        return Derived::is( l_, 1 );
    }

    static int overloaded_function_router( ::lua_State* l_ ) {
        typedef bool( *validate_type )( ::lua_State*, int );
        auto table = lua_upvalueindex( 1 );
        lua_pushnil( l_ );
        while ( lua_next( l_, table ) ) {
            auto& clb = *reinterpret_cast<const validate_type*>( lua_topointer( l_, -2 ) );
            if ( clb( l_, 2 ) ) {
                lua_insert( l_, 1 );
                lua_pop( l_, 1 );
                lua_call( l_, lua_gettop( l_ ) - 1, LUA_MULTRET );
                return lua_gettop( l_ );
            } else {
                lua_pop( l_, 1 );
            }
        }
        return 0;
    }

    static int overloaded_method_router( ::lua_State* l_ ) {
        return overloaded_function_router( l_ );
    }

    static void begin_overloaded_method( ::lua_State* l_, const char* name_ ) {
        lua_pushstring( l_, name_ );
        lua_newtable( l_ );
    }

    template<typename RetType, typename... Args>
    static void add_overloaded_method_variant( ::lua_State* l_, RetType( Class::*method_ )( Args... ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( method_ ) method_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_method_params<Args...>;
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            decltype( auto ) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return inoke_method( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        }, 1 );
        lua_settable( l_, -3 );
    }

    template<typename... Args>
    static void add_overloaded_method_variant( ::lua_State* l_, void( Class::*method_ )( Args... ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( method_ ) method_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_method_params<Args...>;
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            decltype( auto ) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            inoke_method_nr( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
            return 0;
        }, 1 );
        lua_settable( l_, -3 );
    }

    template<typename RetType>
    static void add_overloaded_method_variant( ::lua_State* l_, RetType( Class::*method_ )( ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( method_ ) method_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_method_params_np;
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            decltype( auto ) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return ::lua::push( l_, ( self.*member )( ) );
        }, 1 );
        lua_settable( l_, -3 );
    }

    static void add_overloaded_method_variant( ::lua_State* l_, void( Class::*method_ )( ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( method_ ) method_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_method_params_np;
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_method_params_np( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            ( self.*member )( );
            return 0;
        }, 1 );
        lua_settable( l_, -3 );
    }

    static void end_overloaded_method( ::lua_State* l_, int medthod_table_ ) {
        lua_pushcclosure( l_, overloaded_method_router, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename RetType, typename... Args>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, RetType( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_method_params<Args...>( l_ ) ) {
                return 0;
            }
            decltype(auto) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return inoke_method( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }
    template<typename... Args>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, void( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_method_params<Args...>( l_ ) ) {
                return 0;
            }
            decltype(auto) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            inoke_method_nr( l_, &self, member, 2, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
            return 0;
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }
    template<typename RetType>
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, RetType( Class::*method_ )( ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_method_params_np( l_ ) ) {
                return 0;
            }
            decltype( auto ) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return static_cast<int>( ::lua::push( l_, ( self.*member )( ) ) );
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }
    static void add_method( ::lua_State* l_, const char* name_, int medthod_table_, void( Class::*method_ )( ) ) {
        typedef decltype( method_ ) method_type;
        lua_pushstring( l_, name_ );
        *static_cast<method_type*>( lua_newuserdata( l_, sizeof( method_type ) ) ) = method_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_method_params_np( l_ ) ) {
                return 0;
            }
            decltype(auto) self = Derived::to( l_, 1 );
            auto member = *static_cast<const method_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            ( self.*member )( );
            return 0;
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void begin_overloaded_function( ::lua_State* l_, const char* name_ ) {
        lua_pushstring( l_, name_ );
        lua_newtable( l_ );
    }

    template<typename RetType, typename... Args>
    static void add_overloaded_function_variant( ::lua_State* l_, RetType( *function_ )( Args... ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( function_ ) function_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_function_params<Args...>;
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            return invoke_function( l_, *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) ), 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        }, 1 );
        lua_settable( l_, -3 );
    }

    template<typename... Args>
    static void add_overloaded_function_variant( ::lua_State* l_, void( *function_ )( Args... ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( function_ ) function_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_function_params<Args...>;
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            invoke_function_nr( l_, *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) ), 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
            return 0;
        }, 1 );
        lua_settable( l_, -3 );
    }

    template<typename RetType>
    static void add_overloaded_function_variant( ::lua_State* l_, RetType( *function_ )( ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( function_ ) function_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_function_params_np;
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto func = *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return ::lua::push( l_, func() );
        }, 1 );
        lua_settable( l_, -3 );
    }

    static void add_overloaded_function_variant( ::lua_State* l_, void( *function_ )( ) ) {
        typedef bool( *validator_type )( ::lua_State*, int );
        typedef decltype( function_ ) function_type;
        *static_cast<validator_type*>( lua_newuserdata( l_, sizeof( validator_type ) ) ) = validate_function_params_np;
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto func = *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            func();
            return 0;
        }, 1 );
        lua_settable( l_, -3 );
    }

    static void end_overloaded_function( ::lua_State* l_, int medthod_table_ ) {
        lua_pushcclosure( l_, overloaded_function_router, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename RetType, typename... Args>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, RetType( *function_ )( Args... ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_function_params<Args...>( l_ ) ) {
                return 0;
            }
            return invoke_function( l_, *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) ), 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename... Args>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, void( *function_ )( Args... ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_function_params<Args...>( l_ ) ) {
                return 0;
            }
            invoke_function_nr( l_, *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) ), 1, 1, typename tools::gen_seq<sizeof...( Args )>::type {} );
            return 0;
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename RetType>
    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, RetType( *function_ )( ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_function_params_np( l_ ) ) {
                return 0;
            }
            auto func = *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return ::lua::push( l_, func() );
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void add_function( ::lua_State* l_, const char* name_, int medthod_table_, void( *function_ )( ) ) {
        typedef decltype( function_ ) function_type;
        lua_pushstring( l_, name_ );
        *static_cast<function_type*>( lua_newuserdata( l_, sizeof( function_type ) ) ) = function_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            if ( !validate_function_params_np( l_ ) ) {
                return 0;
            }
            auto func = *static_cast<const function_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            func();
            return 0;
        }, 1 );
        lua_settable( l_, medthod_table_ );
    }

    static void add_raw_function( ::lua_State* l_, const char* name_, int medthod_table_, lua_CFunction func_ ) {
        lua_pushstring( l_, name_ );
        lua_pushcclosure( l_, func_, 0 );
        lua_settable( l_, medthod_table_ );
    }

    template<typename Type>
    static void add_property( ::lua_State* l_, const char* name_, int geter_table_, int seter_table_, Type( Class::*geter_ )( ), void( Class::*seter_ )( Type ) ) {
        typedef decltype( geter_ ) geter_type;
        typedef decltype( seter_ ) seter_type;
        if ( geter_ ) {
            lua_pushstring( l_, name_ );
            *reinterpret_cast<geter_type*>( lua_newuserdata( l_, sizeof( geter_type ) ) ) = geter_;
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                auto& self = Derived::to( l_, 1 );
                auto geter = *reinterpret_cast<const geter_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
                return static_cast<int>( ::lua::push( l_, ( self.*geter )( ) ) );
            }, 1 );
            lua_settable( l_, geter_table_ );
        }

        if ( seter_ ) {
            lua_pushstring( l_, name_ );
            *reinterpret_cast<seter_type*>( lua_newuserdata( l_, sizeof( seter_type ) ) ) = seter_;
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                auto& self = Derived::to( l_, 1 );
                auto seter = *reinterpret_cast<const seter_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
                ( self.*seter )( ::lua::to<Type>( l_, 2 ) );
                return 0;
            }, 1 );
            lua_settable( l_, seter_table_ );
        }
    }

    template<typename Type>
    static void add_static_property( ::lua_State* l_, const char* name_, int geter_table_, int seter_table_, Type( *geter_ )( ), void( *seter_ )( Type ) ) {
        typedef decltype( geter_ ) geter_type;
        typedef decltype( seter_ ) seter_type;
        if ( geter_ ) {
            lua_pushstring( l_, name_ );
            *reinterpret_cast<geter_type*>( lua_newuserdata( l_, sizeof( geter_type ) ) ) = geter_;
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                auto geter = *reinterpret_cast<const geter_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
                return ::lua::push( l_, ( geter )( ) );
            }, 1 );
            lua_settable( l_, geter_table_ );
        }

        if ( seter_ ) {
            lua_pushstring( l_, name_ );
            *reinterpret_cast<seter_type*>( lua_newuserdata( l_, sizeof( seter_type ) ) ) = seter_;
            lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
                auto seter = *reinterpret_cast<const seter_type*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
                ( seter )( ::lua::to<Type>( l_, 2 ) );// index 1 is the function table
                return 0;
            }, 1 );
            lua_settable( l_, seter_table_ );
        }
    }

    template<typename Type>
    static void add_member( ::lua_State* l_, const char* name_, int geter_table_, int seter_table_, Type Class::*member_ ) {
        typedef decltype( member_ ) member_pointer;
        lua_pushstring( l_, name_ );
        *reinterpret_cast<member_pointer*>( lua_newuserdata( l_, sizeof( member_pointer ) ) ) = member_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto& self = Derived::to( l_, 1 );
            auto member = *reinterpret_cast<const member_pointer*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return static_cast<int>( ::lua::push( l_, self.*member ) );
        }, 1 );
        lua_settable( l_, geter_table_ );

        lua_pushstring( l_, name_ );
        *reinterpret_cast<member_pointer*>( lua_newuserdata( l_, sizeof( member_pointer ) ) ) = member_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto& self = Derived::to( l_, 1 );
            auto member = *reinterpret_cast<const member_pointer*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            ( self.*member ) = ::lua::to<Type>( l_, 2 );
            return 0;
        }, 1 );
        lua_settable( l_, seter_table_ );
    }

    template<typename Type>
    static void add_static_member( ::lua_State* l_, const char* name_, int geter_table_, int seter_table_, Type *member_ ) {
        typedef decltype( member_ ) member_pointer;
        lua_pushstring( l_, name_ );
        *reinterpret_cast<member_pointer*>( lua_newuserdata( l_, sizeof( member_pointer ) ) ) = member_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto member = *reinterpret_cast<const member_pointer*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            return ::lua::push( l_, *member );
        }, 1 );
        lua_settable( l_, geter_table_ );

        lua_pushstring( l_, name_ );
        *reinterpret_cast<member_pointer*>( lua_newuserdata( l_, sizeof( member_pointer ) ) ) = member_;
        lua_pushcclosure( l_, []( ::lua_State* l_ ) -> int {
            auto& self = Derived::to( l_, 1 );
            auto member = *reinterpret_cast<const member_pointer*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
            *member = ::lua::to<Type>( l_, 2 );
            return 0;
        }, 1 );
        lua_settable( l_, seter_table_ );
    }

    template<typename Type>
    static void add_constant( ::lua_State* l_, const char* name_, int metatable_, Type value_ ) {
        lua_pushstring( l_, name_ );
        ::lua::push( l_, std::forward<Type>( value_ ) );
        lua_settable( l_, metatable_ );
    }



    static int on_to_string( ::lua_State* l_ ) {
        auto& self = Derived::to( l_, 1 );
        lua_pushfstring( l_, "%s (%08p)", Derived::name, &self );
        return 1;
    }

    static int on_gc( ::lua_State* l_ ) {
        auto& self = Derived::to( l_, 1 );
        Derived::destroy( l_, self );
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