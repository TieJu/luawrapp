#pragma once

extern "C" {
#include <lua.h>
}

#include <atomic>
#include <memory>

namespace lua {
namespace detail {
template<typename Derived>
class context_base {
protected:
    ::lua_State *_l { nullptr };

public:
    context_base() = default;
    ~context_base() { static_cast<Derived*>( this )->close(); }

    context_base( const context_base& other_ ) :_l { static_cast<const Derived*>( &other_ )->copy_handle() } {}
    context_base& operator=( const context_base& other_ ) {
        static_cast<Derived*>( this )->close();
        _l = static_cast<const Derived*>( &other_ )->copy_handle();
        return *this;
    }

    context_base( context_base&& other_ ) :_l { other_.release() } {}
    context_base& operator=( context_base&& other_ ) {
        static_cast<Derived*>( this )->close();
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

    explicit context_base( init init_ ) { static_cast<Derived*>( this )->open( init_ ); }

    ::lua_State* release() {
        auto cpy = _l;
        _l = nullptr;
        return cpy;
    }

    ::lua_State* get() {
        return _l;
    }

    template<typename D>
    void thread_of( context_base<D>& other_ ) {
        static_cast<Derived*>( this )->close();
        _l = lua_newthread( other_.get() );
    }

    void thread_of( ::lua_State* l_ ) {
        static_cast<Derived*>( this )->close();
        _l = lua_newthread( l_ );
    }

    int get_top() {
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
    /*

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);*/

    /*
LUA_API void  (lua_pushnil) (lua_State *L);
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);*/

    /*
LUA_API void  (lua_gettable) (lua_State *L, int idx);
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawget) (lua_State *L, int idx);
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API void  (lua_getfenv) (lua_State *L, int idx);*/

    /*
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setfenv) (lua_State *L, int idx);*/

    /*
LUA_API void  (lua_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int   (lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                                        const char *chunkname);

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data);*/

    /*
LUA_API int  (lua_yield) (lua_State *L, int nresults);
LUA_API int  (lua_resume) (lua_State *L, int narg);
LUA_API int  (lua_status) (lua_State *L);*/

    /*

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7

LUA_API int (lua_gc) (lua_State *L, int what, int data);*/

    /*

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);*/

    /*

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)*/

/*

#define lua_open()	luaL_newstate()

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)

#define lua_getgccount(L)	lua_gc(L, LUA_GCCOUNT, 0)

#define lua_Chunkreader		lua_Reader
#define lua_Chunkwriter		lua_Writer


/* hack */
//LUA_API void lua_setlevel( lua_State *from, lua_State *to ); */

/*
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4*/

/*
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)*/

/*
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);
LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook lua_gethook (lua_State *L);
LUA_API int lua_gethookmask (lua_State *L);
LUA_API int lua_gethookcount (lua_State *L);

LUA_API void *lua_upvalueid( lua_State *L, int idx, int n );
LUA_API void lua_upvaluejoin( lua_State *L, int idx1, int n1, int idx2, int n2 );
LUA_API int lua_loadx( lua_State *L, lua_Reader reader, void *dt,
                       const char *chunkname, const char *mode );
                       
*/
//struct lua_Debug {
//    int event;
//    const char *name;	/* (n) */
//    const char *namewhat;	/* (n) `global', `local', `field', `method' */
//    const char *what;	/* (S) `Lua', `C', `main', `tail' */
//    const char *source;	/* (S) */
//    int currentline;	/* (l) */
//    int nups;		/* (u) number of upvalues */
//    int linedefined;	/* (S) */
//    int lastlinedefined;	/* (S) */
//    char short_src[LUA_IDSIZE]; /* (S) */
//    /* private part */
//    int i_ci;  /* active function */
//}; */
};
}
class context
    : public detail::context_base<context> {
    friend class detail::context_base<context>;

    //::lua_State* copy_handle() const;

public:
    using detail::context_base<context>::context_base;

    void close() {
        if ( _l ) {
            lua_close( _l );
            _l = nullptr;
        }
    }

    void open( init init_ ) {
        close();
        _l = lua_newstate( init_._allocator, init_._data );
    }

    void attach( ::lua_State* l_ ) {
        close();
        _l = l_;
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

public:
    using detail::context_base<shared_context>::context_base;

    void close() {
        if ( _l ) {
            sub_ref();
            _l = nullptr;
        }
    }

    void open( init init_ ) {
        close();
        _l = lua_newstate( init_._allocator, init_._data );
        add_ref();
    }

    void attach( ::lua_State* l_ ) {
        close();
        _l = l_;
        add_ref();
    }
};
}