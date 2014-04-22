#pragma once

namespace lua {
template<typename Derived>
struct stack {
    int get_top() {
        return lua_gettop( static_cast<Derived*>( this )->get() );
    }

    void set_top( int index_ ) {
        lua_settop( static_cast<Derived*>( this )->get(), index_ );
    }

    void push_value( int index_ ) {
        lua_pushvalue( static_cast<Derived*>( this )->get(), index_ );
    }

    void remove( int index_ ) {
        lua_remove( static_cast<Derived*>( this )->get(), index_ );
    }

    void insert( int index_ ) {
        lua_insert( static_cast<Derived*>( this )->get(), index_ );
    }

    void replace( int index_ ) {
        lua_replace( static_cast<Derived*>( this )->get(), index_ );
    }

    void check_stack( int size_ ) {
        lua_checkstack( static_cast<Derived*>( this )->get(), size_ );
    }

    bool is_number( int index_ ) {
        return 0 != lua_isnumber( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_string( int index_ ) {
        return 0 != lua_isstring( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_c_function( int index_ ) {
        return 0 != lua_iscfunction( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_user_data( int index_ ) {
        return 0 != lua_isuserdata( static_cast<Derived*>( this )->get(), index_ );
    }

    int type( int index_ ) {
        return lua_type( static_cast<Derived*>( this )->get(), index_ );
    }

    bool equal( int first_, int second_ ) {
        return  0 != lua_equal( static_cast<Derived*>( this )->get(), first_, second_ );
    }

    bool raw_equal( int first_, int second_ ) {
        return  0 != lua_rawequal( static_cast<Derived*>( this )->get(), first_, second_ );
    }

    bool less_than( int first_, int second_ ) {
        return  0 != lua_lessthan( static_cast<Derived*>( this )->get(), first_, second_ );
    }

    lua_Number to_number( int index_ ) {
        return lua_tonumber( static_cast<Derived*>( this )->get(), index_ );
    }

    lua_Integer to_integer( int index_ ) {
        return lua_tointeger( static_cast<Derived*>( this )->get(), index_ );
    }

    bool to_boolean( int index_ ) {
        return 0 != lua_toboolean( static_cast<Derived*>( this )->get(), index_ );
    }

    const char* to_string( int index_, int& len_ ) {
        return lua_tolstring( static_cast<Derived*>( this )->get(), index_, &len_ );
    }

    const char* to_string( int index_ ) {
        return lua_tolstring( static_cast<Derived*>( this )->get(), index_, nullptr );
    }

    size_t obj_len( int index_ ) {
        return lua_objlen( static_cast<Derived*>( this )->get(), index_ );
    }

    lua_CFunction to_c_function( int index_ ) {
        return lua_tocfunction( static_cast<Derived*>( this )->get(), index_ );
    }

    void* to_user_data( int index_ ) {
        return lua_touserdata( static_cast<Derived*>( this )->get(), index_ );
    }

    lua_State* to_thread( int index_ ) {
        return lua_tothread( static_cast<Derived*>( this )->get(), index_ );
    }

    const void* to_pointer( int index_ ) {
        return lua_topointer( static_cast<Derived*>( this )->get(), index_ );
    }

    void push_nil() {
        lua_pushnil( static_cast<Derived*>( this )->get() );
    }

    void push_number( lua_Number v_ ) {
        lua_pushnumber( static_cast<Derived*>( this )->get(), v_ );
    }

    void push_integer( lua_Integer v_ ) {
        lua_pushinteger( static_cast<Derived*>( this )->get(), v_ );
    }

    void push_string( const char* str_, size_t len_ ) {
        lua_pushlstring( static_cast<Derived*>( this )->get(), str_, len_ );
    }

    void push_string( const char* str_ ) {
        lua_pushstring( static_cast<Derived*>( this )->get(), str_ );
    }

    const char* push_vfstring( const char* fmt_, va_list args_ ) {
        return lua_pushvfstring( static_cast<Derived*>( this )->get(), fmt_, args_ );
    }

    const char* push_fstring( const char* fmt_, ... ) {
        va_list args;
        va_start( args, fmt_ );
        auto result = lua_pushvfstring( fmt_, args );
        va_end( args );
        return result;
    }

    void push_c_closure( lua_CFunction fn_, int params_ ) {
        lua_pushcclosure( static_cast<Derived*>( this )->get(), fn_, params_ );
    }

    void push_boolean( bool boolean_ ) {
        lua_pushboolean( static_cast<Derived*>( this )->get(), boolean_ ? 1 : 0 );
    }

    void push_light_user_data( void* ptr_ ) {
        lua_pushlightuserdata( static_cast<Derived*>( this )->get(), ptr_ );
    }

    void push_thread() {
        lua_pushthread( static_cast<Derived*>( this )->get() );
    }

    void get_table( int index_ ) {
        lua_gettable( static_cast<Derived*>( this )->get(), index_ );
    }

    void get_field( int index_, const char* str_ ) {
        lua_getfield( static_cast<Derived*>( this )->get(), index_, str_ );
    }

    void raw_get( int index_ ) {
        lua_rawget( static_cast<Derived*>( this )->get(), index_ );
    }

    void raw_geti( int index_, int n_ ) {
        lua_rawgeti( static_cast<Derived*>( this )->get(), index_, n_ );
    }

    void create_table( int numbers_ = 0, int records_ = 0 ) {
        lua_createtable( static_cast<Derived*>( this )->get(), numbers_, records_ );
    }

    void* new_user_data( size_t size_ ) {
        return lua_newuserdata( static_cast<Derived*>( this )->get(), size_ );
    }

    bool get_meta_table( int index_ ) {
        return 0 != lua_getmetatable( static_cast<Derived*>( this )->get(), index_ );
    }

    void get_fenv( int index_ ) {
        lua_getfenv( static_cast<Derived*>( this )->get(), index_ );
    }

    void set_table( int index_ ) {
        lua_settable( static_cast<Derived*>( this )->get(), index_ );
    }

    void set_field( int index_, const char* str_ ) {
        lua_setfield( static_cast<Derived*>( this )->get(), index_, str_ );
    }

    void raw_set( int index_ ) {
        lua_rawset( static_cast<Derived*>( this )->get(), index_ );
    }

    void raw_seti( int index_, int n_ ) {
        lua_rawseti( static_cast<Derived*>( this )->get(), index_, n_ );
    }

    void set_meta_table( int index_ ) {
        lua_setmetatable( static_cast<Derived*>( this )->get(), index_ );
    }

    void set_fenv( int index_ ) {
        lua_setfenv( static_cast<Derived*>( this )->get(), index_ );
    }

    void call( int num_args_, int num_results_ ) {
        lua_call( static_cast<Derived*>( this )->get(), num_args_, num_results_ );
    }

    int pcall( int num_args_, int num_results_, int error_func_ ) {
        return lua_pcall( static_cast<Derived*>( this )->get(), num_args_, num_results_, error_func_ );
    }

    int cpcall( lua_CFunction func_, void* ud_ ) {
        return lua_cpcall( static_cast<Derived*>( this )->get(), func_, ud_ );
    }

    int next( int index_ ) {
        return lua_next( static_cast<Derived*>( this )->get(), index_ );
    }

    void concat( int count_ ) {
        lua_concat( static_cast<Derived*>( this )->get(), count_ );
    }

    void pop( int count_ = 1 ) {
        lua_pop( static_cast<Derived*>( this )->get(), count_ );
    }

    void new_table( int numbers_ = 0, int records_ = 0 ) {
        lua_createtable( static_cast<Derived*>( this )->get(), numbers_, records_ );
    }

    void push_c_function( lua_CFunction func_ ) {
        push_c_closure( func_, 0 );
    }

    size_t strlen( int index_ ) {
        return obj_len( index_ );
    }

    bool is_function( int index_ ) {
        return 0 != lua_isfunction( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_table( int index_ ) {
        return 0 != lua_istable( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_light_user_data( int index_ ) {
        return 0 != lua_islightuserdata( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_nil( int index_ ) {
        return 0 != lua_isnil( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_boolean( int index_ ) {
        return 0 != lua_isboolean( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_thread( int index_ ) {
        return 0 != lua_isthread( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_none( int index_ ) {
        return 0 != lua_isnone( static_cast<Derived*>( this )->get(), index_ );
    }

    bool is_none_or_nil( int index_ ) {
        return 0 != lua_isnoneornil( static_cast<Derived*>( this )->get(), index_ );
    }

    template<size_t N>
    void push_literal( const char( &lit_ )[N] ) {
        push_string( lit_, N - 1 );
    }

    void set_global( const char* str_ ) {
        lua_setglobal( static_cast<Derived*>( this )->get(), str_ );
    }

    void get_global( const char* str_ ) {
        lua_getglobal( static_cast<Derived*>( this )->get(), str_ );
    }

    void get_registry() {
        lua_getregistry( static_cast<Derived*>( this )->get() );
    }

    template<typename Type>
    void push( Type value_ ) {
        using ::lua::push;
        push( static_cast<Derived*>( this )->get(), std::forward<Type>( value_ ) );
    }

    template<typename... Args>
    void push( Args... args_ ) {
        using ::lua::push;
        push( static_cast<Derived*>( this )->get(), std::forward<Args>( args_ )... );
    }

    template<typename Type>
    Type to( int index_ ) {
        using ::lua::to;
        return to<Type>( static_cast<Derived*>( this )->get(), index_ );
    }

    template<typename... Args>
    std::tuple<Args...> to( int index_, int step_ ) {
        using ::lua::to;
        return to<Args...>( static_cast<Derived*>( this )->get(), index_, step_ );
    }

    template<typename Type>
    bool is( int index_ ) {
        return lua::type_trait<Type>::is( static_cast<Derived*>( this )->get(), index_ );
    }

    int abs_stack_index( int index_ ) {
        return index_ > 0 ? index_
            : index_ <= LUA_REGISTRYINDEX ? _idx
            : get_top() + 1 + index_;
    }
};
}