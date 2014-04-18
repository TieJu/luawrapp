#pragma once

#include <atomic>
#include <memory>
#include <bitset>
#include <utility>

#include "lua_push.h"
#include "lua_to.h"
#include "lua_var.h"

namespace lua {
namespace detail {
template<typename Derived>
class context_base {
protected:
    ::lua_State *_l { nullptr };

public:
    context_base() = default;
    ~context_base() { static_cast<Derived*>( this )->close_handle(); }

    context_base( const context_base& other_ ) :_l { static_cast<const Derived*>( &other_ )->copy_handle() } {}
    context_base& operator=( const context_base& other_ ) {
        static_cast<Derived*>( this )->close_handle();
        _l = static_cast<const Derived*>( &other_ )->copy_handle();
        return *this;
    }

    context_base( context_base&& other_ ) :_l { other_.release() } {}
    context_base& operator=( context_base&& other_ ) {
        static_cast<Derived*>( this )->close_handle();
        _l = other_.release();
        return *this;
    }

    explicit operator bool() const { return _l != nullptr; }
    bool operator!() const { return _l == nullptr; }

    bool empty() const { return _l == nullptr; }

    struct init {
        lua_Alloc   _allocator { nullptr };
        void*       _data { nullptr };
    };

    explicit context_base( init init_ ) { static_cast<Derived*>( this )->open_handle( init_ ); }
    explicit context_base( ::lua_State* l_ ) { static_cast<Derived*>( this )->attach_handle( l_ ); }

    ::lua_State* release() {
        auto cpy = _l;
        _l = nullptr;
        return cpy;
    }

    ::lua_State* get() const {
        return _l;
    }

    template<typename D>
    void thread_of( context_base<D>& other_ ) {
        static_cast<Derived*>( this )->close_handle();
        static_cast<Derived*>( this )->attach_handle( lua_newthread( other_.get() ) );
    }

    void thread_of( ::lua_State* l_ ) {
        static_cast<Derived*>( this )->close_handle();
        static_cast<Derived*>( this )->attach_handle( lua_newthread( l_ ) );
    }

    int get_top() const {
        return lua_gettop( _l );
    }

    void set_top( int index_ ) {
        lua_settop( _l, index_ );
    }

    void push_value( int index_ ) {
        lua_pushvalue( _l, index_ );
    }

    void remove( int index_ ) {
        lua_remove( _l, index_ );
    }

    void insert( int index_ ) {
        lua_insert( _l, index_ );
    }

    void replace( int index_ ) {
        lua_replace( _l, index_ );
    }

    void check_stack( int size_ ) {
        lua_checkstack( _l, size_ );
    }

    void xmove_to( ::lua_State* l_, int num_ ) {
        lua_xmove( _l, l_, num_ );
    }

    template<typename D>
    void xmove_to( context_base<D>& other_, int num_ ) {
        lua_xmove( _l, other_.get(), num_ );
    }

    void xmove_from( ::lua_State* l_, int num_ ) {
        lua_xmove( l_, _l, num_ );
    }

    template<typename D>
    void xmove_from( context_base<D>& other_, int num_ ) {
        lua_xmove( other_.get(), _l, num_ );
    }

    bool is_number( int index_ ) {
        return 0 != lua_isnumber( _l, index_ );
    }

    bool is_string( int index_ ) {
        return 0 != lua_isstring( _l, index_ );
    }

    bool is_c_function( int index_ ) {
        return 0 != lua_iscfunction( _l, index_ );
    }

    bool is_user_data( int index_ ) {
        return 0 != lua_isuserdata( _l, index_ );
    }

    int type( int index_ ) {
        return lua_type( _l, index_ );
    }

    const char* type_name( int type_ ) {
        return lua_typename( _l, type_ );
    }

    bool equal( int first_, int second_ ) {
        return  0 != lua_equal( _l, first_, second_ );
    }

    bool raw_equal( int first_, int second_ ) {
        return  0 != lua_rawequal( _l, first_, second_ );
    }

    bool less_than( int first_, int second_ ) {
        return  0 != lua_lessthan( _l, first_, second_ );
    }

    lua_Number to_number( int index_ ) {
        return lua_tonumber( _l, index_ );
    }

    lua_Integer to_integer( int index_ ) {
        return lua_tointeger( _l, index_ );
    }

    bool to_boolean( int index_ ) {
        return 0 != lua_toboolean( _l, index_ );
    }

    const char* to_string( int index_, int& len_ ) {
        return lua_tolstring( _l, index_, &len_ );
    }

    const char* to_string( int index_ ) {
        return lua_tolstring( _l, index_, nullptr );
    }

    size_t obj_len( int index_ ) {
        return lua_objlen( _l, index_ );
    }

    lua_CFunction to_c_function( int index_ ) {
        return lua_tocfunction( _l, index_ );
    }

    void* to_user_data( int index_ ) {
        return lua_touserdata( _l, index_ );
    }

    lua_State* to_thread( int index_ ) {
        return lua_tothread( _l, index_ );
    }

    const void* to_pointer( int index_ ) {
        return lua_topointer( _l, index_ );
    }

    void push_nil() {
        lua_pushnil( _l );
    }

    void push_number(lua_Number v_) {
        lua_pushnumber( _l, v_ );
    }

    void push_integer( lua_Integer v_ ) {
        lua_pushinteger( _l, v_ );
    }

    void push_string( const char* str_, size_t len_ ) {
        lua_pushlstring( _l, str_, len_ );
    }

    void push_string( const char* str_ ) {
        lua_pushstring( _l, str_ );
    }

    const char* push_vfstring( const char* fmt_, va_list args_ ) {
        return lua_pushvfstring( _l, fmt_, args_ );
    }

    const char* push_fstring( const char* fmt_, ... ) {
        va_list args;
        va_start( args, fmt_ );
        auto result = lua_pushvfstring( fmt_, args );
        va_end( args );
        return result;
    }

    void push_c_closure( lua_CFunction fn_, int params_ ) {
        lua_pushcclosure( _l, fn_, params_ );
    }

    void push_boolean( bool boolean_ ) {
        lua_pushboolean( _l, boolean_ ? 1 : 0 );
    }

    void push_light_user_data( void* ptr_ ) {
        lua_pushlightuserdata( _l, ptr_ );
    }

    void push_thread() {
        lua_pushthread( _l );
    }

    void get_table( int index_ ) {
        lua_gettable( _l, index_ );
    }

    void get_field( int index_, const char* str_ ) {
        lua_getfield( _l, index_, str_ );
    }

    void raw_get( int index_ ) {
        lua_rawget( _l, index_ );
    }

    void raw_geti( int index_ ) {
        lua_rawgeti( _l, index_ );
    }

    void create_table( int numbers_ = 0, int records_ = 0 ) {
        lua_createtable( _l, numbers_, records_ );
    }

    void* new_user_data( size_t size_ ) {
        return lua_newuserdata( _l, size_ );
    }

    bool get_meta_table( int index_ ) {
        return 0 != lua_getmetatable( _l, index_ );
    }

    void get_fenv( int index_ ) {
        lua_getfenv( _l, index_ );
    }

    void set_tabe( int index_ ) {
        lua_settable( _l, index_ );
    }

    void set_field( int index_, const char* str_ ) {
        lua_setfield( _l, index_, str_ );
    }

    void raw_set( int index_ ) {
        lua_rawset( _l, index_ );
    }

    void raw_seti( int index_, int n_ ) {
        lua_rawseti( _l, index_, n_ );
    }

    void set_meta_table( int index_ ) {
        lua_setmetatable( _l, index_ );
    }

    void set_fenv( int index_ ) {
        lua_setfenv( _l, index_ );
    }

    void call( int num_args_, int num_results_ ) {
        lua_call( _l, num_args_, num_results_ );
    }

    int pcall( int num_args_, int num_results_, int error_func_ ) {
        return lua_pcall( _l, num_args_, num_results_, error_func_ );
    }

    int cpcall( lua_CFunction func_, void* ud_ ) {
        return lua_cpcall( _l, func_, ud_ );
    }

    int load( lua_Reader reader_, void* data_, const char* name_ ) {
        return lua_load( _l, reader_, data_, name_ );
    }

    int dump( lua_Writer writer_, void* data_ ) {
        return lua_dump( _l, writer_, data_ );
    }

    int yield( int num_results_ ) {
        return lua_yield( _l, num_results_ );
    }

    int resume( int num_args_ ) {
        return lua_resume( _l, num_args_ );
    }

    int status() {
        return lua_status( _l );
    }

    enum class gc_param {
        stop = LUA_GCSTOP,
        restart = LUA_GCRESTART,
        collect = LUA_GCCOLLECT,
        count = LUA_GCCOUNT,
        count_bytes = LUA_GCCOUNTB,
        step = LUA_GCSTEP,
        pause = LUA_GCSETPAUSE,
        step_mul = LUA_GCSETSTEPMUL
    };
    int gc( gc_param param_, int data_ ) {
        return lua_gc( _l, int( param_ ), data_ );
    }

    int error() {
        return lua_error( _l );
    }

    int next( int index_ ) {
        return lua_next( _l, index_ );
    }

    void concat( int count_ ) {
        lua_concat( _l, count_ );
    }

    lua_Alloc get_alloc_f( void*& user_data_ ) {
        return lua_getallocf( _l, &user_data_ );
    }

    void set_alloc_f( lua_Alloc func_, void* user_data_ ) {
        lua_setallocf( _l, func_, user_data_ );
    }

    void pop( int count_ = 1 ) {
        lua_pop( _l, count_ );
    }

    void new_table( int numbers_ = 0, int records_ = 0 ) {
        lua_createtable( _l, numbers_, records_ );
    }

    void registr( const char* nane_, lua_CFunction func_ ) {
        lua_register( _l, name_, func_ );
    }

    void push_c_function( lua_CFunction func_ ) {
        push_c_closure( func_, 0 );
    }

    size_t strlen( int index_ ) {
        return obj_len( index_ );
    }

    bool is_function( int index_ ) {
        return 0 != lua_isfunction( _l, index_ );
    }

    bool is_table( int index_ ) {
        return 0 != lua_istable( _l, index_ );
    }

    bool is_light_user_data( int index_ ) {
        return 0 != lua_islightuserdata( _l, index_ );
    }

    bool is_nil( int index_ ) {
        return 0 != lua_isnil( _l, index_ );
    }

    bool is_boolean( int index_ ) {
        return 0 != lua_isboolean( _l, index_ );
    }

    bool is_thread( int index_ ) {
        return 0 != lua_isthread( _l, index_ );
    }

    bool is_none( int index_ ) {
        return 0 != lua_isnone( _l, index_ );
    }

    bool is_none_or_nil( int index_ ) {
        return 0 != lua_isnoneornil( _l, index_ );
    }

    template<size_t N>
    void push_literal( const char (&lit_) [N] ) {
        push_string( lit_, N - 1 );
    }

    void set_global( const char* str_ ) {
        lua_setglobal( _l, str_ );
    }

    void get_global( const char* str_ ) {
        lua_getglobal( _l, str_ );
    }

    void get_registry() {
        lua_getregistry( _l );
    }

    int get_gc_count() {
        return lua_getgccount( _l );
    }

    struct debug_event {
    enum {
        call = LUA_HOOKCALL,
        ret = LUA_HOOKRET,
        line = LUA_HOOKLINE,
        count = LUA_HOOKCOUNT,
        tail_ret = LUA_HOOKTAILRET
    };
    };

    typedef std::bitset<5> debug_mask;

    bool get_stack( int level_, lua_Debug& dbg_ ) {
        return 0 != lua_getstack( _l, level_, &dbg_ );
    }

    bool get_info( const char* what_, lua_Debug& dbg_ ) {
        return 0 != lua_getinfo( _l, what_, dbg_ );
    }

    const char* get_local( const lua_Debug& dbg_, int n_ ) {
        return lua_getlocal( _l, &dbg_, n_ );
    }

    const char* set_local( const lua_Debug& dbg_, int n_ ) {
        return lua_setlocal( _l, &dbg_, n_ );
    }

    const char* get_upvalue( int func_, int n_ ) {
        return lua_getupvalue( _l, func_, n_ );
    }

    const char* set_upvalue( int func_, int n_ ) {
        return lua_setupvalue( _l, func_, n_ );
    }

    int set_hook( lua_Hook hook_, debug_mask mask_, int count_ ) {
        return lua_sethook( _l, hook_, mask_.to_ulong(), count_ );
    }

    lua_Hook get_hook() {
        return lua_gethook( _l );
    }

    debug_mask get_hook_mask() {
        return { lua_gethookmask( _l ) };
    }

    int get_hook_count() {
        return lua_gethookcount( _l );
    }

    template<typename Type>
    void push( Type value_ ) {
        using ::lua::push;
        push( _l, std::forward<Type>( value_ ) );
    }

    template<typename... Args>
    void push( Args... args_ ) {
        using ::lua::push;
        push( _l, std::forward<Args>( args_ )... );
    }

    template<typename Type>
    Type to( int index_ ) const {
        using ::lua::to;
        return to<Type>( _l, index_ );
    }

    template<typename... Args>
    std::tuple<Args...> to( int index_, int step_ ) {
        using ::lua::to;
        return to<Args...>( _l, index_, step_ );
    }

    template<typename Type>
    bool is( int index_ ) {
        return lua::type_trait<Type>::is( _l, index_ );
    }

    int abs_stack_index( int index_ ) const {
        return index_ > 0 ? index_
             : index_ <= LUA_REGISTRYINDEX ? _idx
             : get_top() + 1 + index_;
    }

    var<Derived&, stack_index_table, stack_index> get_at_stack( int index_ ) {
        return var<Derived&, stack_index_table, stack_index>{*static_cast<Derived*>( this ), {}, { index_ } };
    }

    template<typename Type>
    var<Derived&, stack_index_table, stack_index> push_to_stack( Type value_ ) {
        push( std::forward<Type>( value_ ) );
        return get_at_stack( get_top() );
    }

    template<typename Type>
    var<Derived&, raw_static_index_table<LUA_GLOBALSINDEX>, var_as_index<stack_index_table, stack_index>> get_global_var( Type value_ ) {
        auto index = push_to_stack( value_ );
        return var<Derived&, raw_static_index_table<LUA_GLOBALSINDEX>, var_as_index<stack_index_table, stack_index>> {*static_cast<Derived*>( this ), {}, { index.table(), index.index() } };
    }

    stack_marker<Derived&> mark_stack() {
        return { *static_cast<Derived*>( this ) };
    }
};

template<typename Derived>
inline bool operator==( const context_base<Derived>& lhv_, const context_base<Derived>& rhv_ ) {
    return lhv_.get() == rhv_.get();
}

inline constexpr unsigned make_debug_mask() {
    return 0;
}

template<typename Value, typename... Values>
inline constexpr unsigned make_debug_mask( Value value_, Values... values_ ) {
    return ( 1 << value_ ) | make_debug_mask( values_... );
}
}
template<typename... Values>
inline std::bitset<5> make_debug_mask( Values... values_ ) {
    return { detail::make_debug_mask( values_... ) };
}

class unique_context
    : public detail::context_base<unique_context> {
    friend class detail::context_base<unique_context>;

    ::lua_State* copy_handle() const = delete;

    void close_handle() {
        if ( _l ) {
            lua_close( _l );
            _l = nullptr;
        }
    }

    void open_handle( init init_ ) {
        close_handle();
        if ( init_._allocator ) {
            _l = lua_newstate( init_._allocator, init_._data );
        } else {
            _l = luaL_newstate();
        }
    }

    void attach_handle( ::lua_State* l_ ) {
        close_handle();
        _l = l_;
    }

public:
    using detail::context_base<unique_context>::context_base;

    void close() {
        close_handle();
    }

    void open( init init_ ) {
        open_handle( init_ );
    }

    void attach( ::lua_State* l_ ) {
        attach_handle( l_ );
    }
};

class context
    : public detail::context_base<context> {
    friend class detail::context_base<context>;

    ::lua_State* copy_handle() const { return _l; }
    
    void close_handle() {
        _l = nullptr;
    }

    inline void open_handle( init init_ ) {
        close_handle();
        if ( init_._allocator ) {
            _l = lua_newstate( init_._allocator, init_._data );
        } else {
            _l = luaL_newstate();
        }
    }

    void attach_handle( ::lua_State* l_ ) {
        close_handle();
        _l = l_;
    }

public:
    using detail::context_base<context>::context_base;
    //context( const context & ) = delete;
    //context& operator=( const context& ) = delete;

    void close() {
        if ( _l ) {
            lua_close( _l );
            _l = nullptr;
        }
    }

    void open( init init_ ) {
        close();
        open_handle( init_ );
    }

    void attach( ::lua_State* l_ ) {
        close();
        attach_handle( l_ );
    }
};

class shared_context
    : public detail::context_base<shared_context> {
    friend class detail::context_base<shared_context>;

    struct shared_store {
        std::atomic_size_t  _ref;
    };

    shared_store* get_store() const {
        auto store = read_store();
        if ( !store ) {
            store = install_store();
        }
        return store;
    }

    shared_store* read_store() const {
        if ( !_l ) {
            return nullptr;
        }
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( _l ) );
        lua_rawget( _l, LUA_REGISTRYINDEX );
        if ( lua_isnil( _l, -1 ) ) {
            lua_pop( _l, 1 );
            return nullptr;
        }

        auto data = reinterpret_cast<shared_store*>( lua_touserdata( _l, -1 ) );
        lua_pop( _l, 1 );

        return data;
    }

    shared_store* install_store() const {
        if ( !_l ) {
            return nullptr;
        }

        auto info = std::make_unique<shared_store>();
        info->_ref = 0;
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( _l ) );
        lua_rawget( _l, LUA_REGISTRYINDEX );
        if ( !lua_isnil( _l, -1 ) ) {
            lua_pop( _l, 1 );
            return nullptr;
        }

        auto ptr = info.get();

        lua_pop( _l, 1 );
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( _l ) );
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( info.release() ) );
        lua_rawset( _l, LUA_REGISTRYINDEX );

        return ptr;
    }

    void remove_store() const {
        if ( !_l ) {
            return;
        }
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( _l ) );
        lua_rawget( _l, LUA_REGISTRYINDEX );
        if ( lua_isnil( _l, -1 ) ) {
            lua_pop( _l, 1 );
            return;
        }
        std::unique_ptr<shared_store> info( reinterpret_cast<shared_store *>( lua_touserdata( _l, -1 ) ) );
        lua_pop( _l, 1 );
        lua_pushlightuserdata( _l, reinterpret_cast<void*>( _l ) );
        lua_pushnil( _l );
        lua_rawset( _l, LUA_REGISTRYINDEX );
    }

    void add_ref() const {
        get_store()->_ref++;
    }

    void sub_ref() {
        if ( !--get_store()->_ref ) {
            remove_store();
            lua_close( _l );
            _l = nullptr;
        }
    }

    ::lua_State* copy_handle() const {
        add_ref();
        return _l;
    }

    void close_handle() {
        if ( _l ) {
            sub_ref();
            _l = nullptr;
        }
    }

    void open_handle( init init_ ) {
        close_handle();
        if ( init_._allocator ) {
            _l = lua_newstate( init_._allocator, init_._data );
        } else {
            _l = luaL_newstate();
        }
        add_ref();
    }

    void attach_handle( ::lua_State* l_ ) {
        close_handle();
        _l = l_;
        add_ref();
    }

public:
    using detail::context_base<shared_context>::context_base;

    void close() {
        close_handle();
    }

    void open( init init_ ) {
        open_handle( init_ );
    }

    void attach( ::lua_State* l_ ) {
        attach_handle( l_ );
    }
};
}