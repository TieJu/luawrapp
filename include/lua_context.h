#pragma once

#include <atomic>
#include <memory>
#include <utility>

namespace lua {
namespace detail {
template<typename Derived>
class context_base
    : public stack_base<context_base<Derived>>
    , public garbage_collector<context_base<Derived>>
    , public debug<context_base<Derived>> {
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

    const char* type_name( int type_ ) {
        return lua_typename( _l, type_ );
    }

    void push_thread() {
        lua_pushthread( _l );
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

    int error() {
        return lua_error( _l );
    }

    lua_Alloc get_alloc_f( void*& user_data_ ) {
        return lua_getallocf( _l, &user_data_ );
    }

    void set_alloc_f( lua_Alloc func_, void* user_data_ ) {
        lua_setallocf( _l, func_, user_data_ );
    }

    template<typename Type>
    void reg_type() {
        type_trait<Type>::reg_type( _l );
    }
    
    template<typename Key, typename Value>
    inline void set_table_entry( int table_id_, Key key_, Value value_ ) {
        ::lua::set_table( get(), table_id_, std::forward<Key>( key_ ), std::forward<Value>( value_ ) );
    }

    template<typename Key>
    inline int get_table_entry( int table_id_, Key key_ ) {
        ::lua::get_table( get(), table_id_, std::forward<Key>( key_ ) );
        return get_top();
    }
};

template<typename Derived>
inline bool operator==( const context_base<Derived>& lhv_, const context_base<Derived>& rhv_ ) {
    return lhv_.get() == rhv_.get();
}
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

    stack<unique_context&> stack() {
        return {*this};
    }

    shared_var<unique_context&> share( int index_ ) {
        return { *this, this->abs_stack_index( index_ ) };
    }

    debug_context<unique_context&> debug() {
        return { ( *this ) };
    }
    
    template<typename KeyType>
    global_var<unique_context&> get_global_var( KeyType key_ ) {
        return { *this, std::forward<KeyType>( key_ ) };
    }

    stack_var<unique_context&> var_at( int index_ ) {
        return { *this, index_ };
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

    stack<context> stack() {
        return {*this};
    }

    shared_var<context> share( int index_ ) {
        return { *this, this->abs_stack_index( index_ ) };
    }

    debug_context<context> debug() {
        return { ( *this ) };
    }

    template<typename KeyType>
    global_var<context> get_global_var( KeyType key_ ) {
        return { *this, std::forward<KeyType>( key_ ) };
    }

    stack_var<context> var_at( int index_ ) {
        return { *this, index_ };
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

    stack<shared_context> stack() {
        return {*this};
    }

    shared_var<shared_context> share( int index_ ) {
        return { *this, this->abs_stack_index( index_ ) };
    }

    debug_context<shared_context> debug() {
        return { ( *this ) };
    }

    template<typename KeyType>
    global_var<shared_context> get_global_var( KeyType key_ ) {
        return { *this, std::forward<KeyType>( key_ ) };
    }

    stack_var<shared_context> var_at( int index_ ) {
        return { *this, index_ };
    }
};
}