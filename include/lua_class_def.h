#pragma once

#include <vector>
#include <unordered_map>

namespace lua {
template<typename State, typename Class>
class class_def {
    State   _state;
    int     _methods;

    struct class_info {
        std::string                                 _name;
        std::vector<Class* ( *)( ::lua_State* l_ )> _contructor_list;
    };

    static class_info& get_def() {
        static class_info info {};
        return info;
    }
    template<typename Tup, int... Seq>
    static Class* invoke_constructor( ::lua_State* l_, Tup&& tup_, tools::seq<Seq...> ) {
        return new ( lua_newuserdata( l_, sizeof( Class ) ) ) Class( std::get<Seq>( tup_ )... );
    }

    static void setup_class( ::lua_State* l_, Class* inst_ ) {
        auto& def = get_def();
        luaL_getmetatable( l_, def._name.c_str() );
        lua_setmetatable( l_, -2 );
    }

    static int construct_class( ::lua_State* l_ ) {
        auto& def = get_def();
        Class *inst = nullptr;
        for ( auto ctr : def._contructor_list ) {
            inst = ctr( l_ );
            if ( inst ) {
                break;
            }
        }

        if ( inst ) {
            setup_class( l_, inst );
            return 1;
        } else {
            luaL_error( l_, "no matching constructor found" );
            return 0;
        }
    }

    void begin( const char* name_ ) {
        auto& info = get_def();
        info._name = name_;
        if ( !_state.is_table( -1 ) ) {
            _state.push_value( LUA_GLOBALSINDEX );
        }
        auto target = _state.get_top();
        auto L = _state.get();
        lua_newtable( L );
        _methods = lua_gettop( L );

        luaL_newmetatable( L, name_ );
        int metatable = lua_gettop( L );

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushstring( L, name_ );
        lua_pushvalue( L, _methods );
        lua_settable( L, target );

        lua_pushliteral( L, "__metatable" );
        lua_pushvalue( L, _methods );
        lua_settable( L, metatable );  // hide metatable from Lua getmetatable()

        lua_pushliteral( L, "__index" );
        lua_pushvalue( L, _methods );
        lua_settable( L, metatable );
        /*
        lua_pushliteral( L, "__tostring" );
        lua_pushcfunction( L, tostring_T );
        lua_settable( L, metatable );
        */
        /*
        lua_pushliteral( L, "__gc" );
        lua_pushcfunction( L, gc_T );
        lua_settable( L, metatable );
        */

        lua_newtable( L );                // mt for method table
        int mt = lua_gettop( L );
        lua_pushliteral( L, "__call" );
        lua_pushcfunction( L, construct_class );
        lua_pushliteral( L, "new" );
        lua_pushvalue( L, -2 );           // dup new_T function
        lua_settable( L, _methods );       // add new_T to method table
        lua_settable( L, mt );            // mt.__call = new_T
        lua_setmetatable( L, _methods );

        //// fill method table with methods from class T
        //for ( RegType *l = T::methods; l->name; l++ ) {
        //    /* edited by Snaily: shouldn't it be const RegType *l ... ? */
        //    lua_pushstring( L, l->name );
        //    lua_pushlightuserdata( L, (void*)l );
        //    lua_pushcclosure( L, thunk, 1 );
        //    lua_settable( L, methods );
        //}

        /*
        luaL_newmetatable( _state.get(), name_ );
        _state.pop();

        _state.new_table();

        _state.push( name_ );
        _state.push_value( -2 );
        _state.set_table( -4 );
        
        _state.push( "new" );
        _state.push( &construct_class );
        _state.set_table( -3 );
        */
    }
    
    template<typename MethodPointer, typename Tup, int... Seq>
    static void inoke_method( ::lua_State* l_, Class* self_, MethodPointer mp_, Tup&& tup_, tools::seq<Seq...> ) {
        push( l_, ( self_->*mp_ )( std::get<Seq>( tup_ )... ) );
    }

    template<typename MethodPointer, typename Tup, int... Seq>
    static void inoke_method_v( ::lua_State* l_, Class* self_, MethodPointer mp_, Tup&& tup_, tools::seq<Seq...> ) {
        ( self_->*mp_ )( std::get<Seq>( tup_ )... );
    }

    template<typename Ret, typename... Args>
    static int call_method( ::lua_State *l_ ) {
        typedef Ret( Class::*pf )( Args... );
        auto mp = *static_cast<const pf*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        auto self = reinterpret_cast<Class*>( const_cast<void*>( lua_topointer( l_, 1 ) ) );
        auto params = to<Args...>( l_, 1, 1 );
        inoke_method( l_, self, mp, , typename tools::gen_seq<sizeof...( Args )>::type {} );
        return 1;
    }

    template<typename... Args>
    static int call_method_v( ::lua_State *l_ ) {
        typedef void( Class::*pf )( Args... );
        auto mp = *static_cast<const pf*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        auto self = reinterpret_cast<Class*>( const_cast<void*>( lua_topointer( l_, 1 ) ) );
        auto params = to<Args...>( l_, 1, 1 );
        inoke_method_v( l_, self, mp, params, typename tools::gen_seq<sizeof...( Args )>::type {} );
        return 0;
    }

    template<typename Ret>
    static int call_method_vr( ::lua_State *l_ ) {
        typedef Ret( Class::*pf )();
        auto mp = *static_cast<const pf*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        auto self = reinterpret_cast<Class*>( const_cast<void*>( lua_topointer( l_, 1 ) ) );
        push( l_, (self->*mp)() );
        return 1;
    }

    static int call_method_vr_v( ::lua_State *l_ ) {
        typedef void( Class::*pf )( );
        auto mp = *static_cast<const pf*>( lua_topointer( l_, lua_upvalueindex( 1 ) ) );
        auto self = reinterpret_cast<Class*>( const_cast<void*>( lua_topointer( l_, 1 ) ) );
        (self->*mp)();
        return 0;
    }

public:
    class_def( const char* name_, State state_ ) : _state { std::move( state_ ) } {
        begin( name_ );
    }

    class_def<State, Class>& constructor() {
        get_def()._contructor_list.push_back( []( ::lua_State* l_ ) -> Class* {
            if ( 0 != lua_gettop( l_ ) ) {
                return nullptr;
            }
            return new ( lua_newuserdata( l_, sizeof( Class ) ) ) Class;
        } );
        return *this;
    }

    template<typename... Args>
    class_def<State, Class>& constructor() {
        get_def()._contructor_list.push_back( []( ::lua_State* l_ ) -> Class* {
            if ( sizeof...( Args ) != lua_gettop( l_ ) ) {
                return nullptr;
            }
            if ( !is<Args...>( l_, 1, 1 ) ) {
                return nullptr;
            }
            auto params = to<Args...>( l_, 1, 1 );
            return invoke_constructor( l_, std::move( params ), typename tools::gen_seq<sizeof...( Args )>::type {} );
        });
        return *this;
    }

    template<typename RetType, typename... Args>
    class_def<State, Class>& method( const char* name_, RetType( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) fp;
        _state.push( name_ );
        new _state.new_user_data( sizeof( fp ) ) fp { method_ };
        _state.push_c_closure( call_method<RetType,Args...>, 1 );
        _state.set_table( _methods );
        return *this;
    }

    template<typename... Args>
    class_def<State, Class>& method( const char* name_, void( Class::*method_ )( Args... ) ) {
        typedef decltype( method_ ) fp;
        _state.push( name_ );
        new (_state.new_user_data( sizeof( fp ) )) fp { method_ };
        _state.push_c_closure( call_method_v<Args...>, 1 );
        _state.set_table( _methods );
        return *this;
    }

    template<typename RetType>
    class_def<State, Class>& method( const char* name_, RetType( Class::*method_ )() ) {
        typedef decltype( method_ ) fp;
        _state.push( name_ );
        new _state.new_user_data( sizeof( fp ) ) fp { method_ };
        _state.push_c_closure_vr( call_method<RetType>, 1 );
        _state.set_table( _methods );
        return *this;
    }

    class_def<State, Class>& method( const char* name_, void( Class::*method_ )() ) {
        typedef decltype( method_ ) fp;
        _state.push( name_ );
        new ( _state.new_user_data( sizeof( fp ) ) ) fp { method_ };
        _state.push_c_closure( call_method_vr_v, 1 );
        _state.set_table( _methods );
        return *this;
    }

    template<typename RetType, typename... Args>
    class_def<State, Class>& function( const char* name_, RetType( *func_ )( Args... ) ) {
        // nothing special to do here, push will handle the wrapping for us
        _state.push( name_ );
        _state.push( func_ );
        _state.set_table( _methods );
        return *this;
    }

    template<typename Type>
    class_def<State, Class>& property( const char* name_, Type( Class::*geter_ )( ), void( Class::*seter_ )( Type ) = nullptr ) {
        return *this;
    }

    template<typename Type>
    class_def<State, Class>& member( const char* name_, Type Class::* member_ ) {
        return *this;
    }

    template<typename Type>
    class_def<State, Class>& constant( const char* name_, Type value_ ) {
        return *this;
    }

    void end() {
        // pop target table
        _state.pop(2);
        // save some space
        get_def()._contructor_list.shrink_to_fit();
    }
};
}