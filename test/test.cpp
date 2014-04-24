#include "../include/luawrapp.h"

#include <utility>

#include <stdio.h>
#include <vector>
#include <type_traits>
#include <varargs.h>
#include <functional>

#include <Windows.h>

struct test_context {
    HANDLE con;
    unsigned started { 0 };
    unsigned failed { 0 };
    unsigned passed { 0 };
    std::vector<std::string> _info_log;
    void write_info_log() {
        SetConsoleTextAttribute( con, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED );
        for ( auto& e : _info_log ) {
            printf( "[info]: %s\r\n", e.c_str() );
        }

    }
    test_context() {
        con = GetStdHandle( STD_OUTPUT_HANDLE );
    }
    void begin_test( const char* name_ ) {
        _info_log.clear();
        SetConsoleTextAttribute( con, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED );
        printf( "test %s ...", name_ );
        ++started;
    }

    void test_ok() {
        SetConsoleTextAttribute( con, FOREGROUND_GREEN );
        printf( "%s\r\n", "ok" );
        ++passed;
        write_info_log();
    }

    void test_failed() {
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", "failed" );
        ++failed;
        write_info_log();
    }

    void test_failed( const char* msg_ ) {
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", "failed" );
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", msg_ );
        ++failed;
        write_info_log();
    }

    ~test_context() {
        SetConsoleTextAttribute( con, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED );
        printf( "=============\r\nTests run: %d\r\n", started );
        SetConsoleTextAttribute( con, FOREGROUND_GREEN );
        printf( "Tests passed: %d\r\n", passed );
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "Tests failed: %d\r\n", failed );
        SetConsoleTextAttribute( con, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED );
        printf( "%s", "press a key to exit" );
        getchar();
    }

    void info_log( const char* fmt_, ... ) {
        va_list arg;
        va_start( arg, fmt_ );
        static char buffer[1024 * 4];
        vsprintf_s( buffer, fmt_, arg );
        _info_log.emplace_back( buffer );
        va_end( arg );
    }
};

#define INFO_LOG(fmt_, ...) __tctx__.info_log(fmt_,__VA_ARGS__)

#define EXPECT_TRUE(test_, info_)\
    if ( !(bool)(test_)) { throw std::exception{"'"info_ "' expected '"#test_"' to be true"}; }
#define EXPECT_FALSE(test_, info_)\
    if ( (bool)(test_)) { throw std::exception{"'"info_ "' expected '"#test_"' to be false"}; }

struct test_token {
    virtual ~test_token() = default;
    virtual void run( test_context& __tctx__ ) = 0;
};

template<typename Test>
struct test_instance : test_token {
    Test _instance;
    virtual void run( test_context& __tctx__ ) override {
        __tctx__.begin_test( _instance.name() );
        try {
            _instance.run_test( __tctx__ );
            __tctx__.test_ok();
        } catch ( const std::exception& e_ ) {
            __tctx__.test_failed( e_.what() );
        } catch ( ... ) {
            __tctx__.test_failed();
        }
    }
};

std::vector<std::unique_ptr<test_token>> _test_set;

void do_all_tests() {
    test_context ctx {};
    for ( auto& test : _test_set ) {
        test->run(ctx);
    }
}

template<typename RegTest>
test_token* reg_test() {
    auto ptr = new test_instance<RegTest> {};
    _test_set.emplace_back( ptr );
    return ptr;
}

#define TestClassName(case_,name_) case_##name_##_test
#define Test(case_,name_) \
struct TestClassName( case_, name_ ) {\
    const char* name() { return #case_": "#name_; }\
    void run_test(test_context& __tctx__);\
    static test_token* _tok;\
};\
    test_token* TestClassName( case_, name_ )::_tok = reg_test<TestClassName( case_, name_ )>();\
    void TestClassName( case_, name_ )::run_test(test_context& __tctx__)

Test( context, unique ) {
    lua::unique_context ctx;
    ctx.open( {} );
    EXPECT_TRUE( ctx.get() != nullptr, "context opened" );
}

Test( context, shared ) {
    lua::shared_context ctx;
    ctx.open( {} );

    auto copy = ctx;

    ctx.close();
    EXPECT_FALSE( ctx.get() != nullptr, "context opened" );
}

test_context* gctx;
void test_print( const char *str_, int value_, float f_ ) {
    test_context& __tctx__ = *gctx;
    EXPECT_TRUE( 0 == strcmp( str_, "test" ), "first param" );
    EXPECT_TRUE( value_ == 8, "second param" );
    EXPECT_TRUE( f_ == 9, "third value" );
}

Test( context, test_function_wrap ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    ctx.push( "print" );
    ctx.push( test_print );
    ctx.set_table( LUA_GLOBALSINDEX );
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), "print(\"test\", 8, 9)" ), "invokation of test_print" );
}

Test( context, var_wrapper ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    auto marker = ctx.mark_stack();
    auto global = ctx.get_global_var( "print" );
    global.set( test_print );
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), "print(\"test\", 8, 9)" ), "invokation of test_print" );
}

Test( context, call_test ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    {
        auto marker = ctx.mark_stack();
        auto global = ctx.get_global_var( "print" );
        global.set( test_print );
    }

    {
        auto marker = ctx.mark_stack(); 
        auto global = ctx.get_global_var( "print" );
        int pos = global.push();
        lua::call( ctx.get(), "test", 8, 9 );
    }
}

Test( context, test_stack_obj ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    {
        auto stack = ctx.begin_stack_block();
        stack.push( "print" );
        stack.push( test_print );
        stack.set_table( LUA_GLOBALSINDEX );
        EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), "print(\"test\", 8, 9)" ), "invokation of test_print" );
    }
}

struct export_test_class {
    std::string _name_var;
    int         _prop_int { 0 };
    int get_int() { return _prop_int; }
    void set_int( int i_ ) { _prop_int = i_; }

    void print() {
        gctx->info_log( "export_test_class at %p { %s; %d }", this, _name_var.c_str(), _prop_int );
    }

    static void static_test() {
        gctx->info_log( "static_test called" );
    }

    export_test_class() = default;
    export_test_class( const char* name_, int i_ ) : _name_var { name_ }, _prop_int { i_ } {}
};

Test( context, open ) {
    lua::unique_context ctx;
    ctx.open( {} );

    EXPECT_TRUE( ctx.get() != nullptr, "valid lua context" );
}

Test( context, load_std_lib ) {
    lua::unique_context ctx;
    ctx.open( {} );

    luaL_openlibs( ctx.get() );
    auto test_code = "print(5)";
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), test_code ), "openlibs" );
}

static int test_function( lua_State*  ) {
    gctx->info_log( __FUNCTION__" called" );
    return 0;
}

Test( context, register_function ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    ctx.push( test_function );
    ctx.set_global( "test" );
    auto test_code = "test()";
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), test_code ), "call user function" );
}

static int script_called_function( lua_State* l_ ) {
    lua_pushliteral( l_, "ok" );
    return 1;
}

Test( context, call_script ) {
    lua::unique_context ctx;
    ctx.open( {} );

    ctx.push( script_called_function );
    ctx.set_global( "test" );
    ctx.get_global( "test" );
    ctx.call( 0, 1 );
    EXPECT_TRUE( ctx.is_string( -1 ), "calling test function" );
}

struct register_class_test_class {
    std::string _name;
    register_class_test_class( const char* name_ ) : _name { name_ } {}
    void print() {
        gctx->info_log( "test at %p = %s", this, _name.c_str() );
    }
    void set_name( const char* name_ ) {
        _name = name_;
    }
    const char* get_name() {
        return _name.c_str();
    }
    void set_name2( const char* name_, int second_, const char* third_ ) {
        _name = name_;
        _name += std::to_string( second_ );
        _name += third_;
    }

    void set( int i_ ) {
        _name = std::to_string( i_ );
    }

    void set( const char* str_ ) {
        _name = str_;
    }

    void name_set( const char* name_ ) {
        _name = name_;
    }

    const char* name_get() {
        return _name.c_str();
    }
};

namespace lua {
template<>
struct type_trait<register_class_test_class> : class_trait_base<register_class_test_class, type_trait<register_class_test_class>> {
    static const char*          name;

    static register_class_test_class* create( lua_State* l_ ) {
        if ( lua_gettop( l_ ) == 0 ) {
            return new ( lua_newuserdata( l_, sizeof( register_class_test_class ) ) ) register_class_test_class { "" };
        } else if ( lua_gettop( l_ ) == 1 && lua_isstring( l_, 1 ) ) {
            return new ( lua_newuserdata( l_, sizeof( register_class_test_class ) ) ) register_class_test_class { lua_tostring( l_, 1 ) };
        } else {
            return nullptr;
        }
    }

    static void destroy( ::lua_State* /*l_*/, register_class_test_class& self_ ) {
        self_.~register_class_test_class();
    }

    static bool is( ::lua_State* l_, int index_ ) {
        return nullptr != luaL_checkudata( l_, index_, name );
    }

    static register_class_test_class& to( ::lua_State* l_, int index_ ) {
        auto self = static_cast<register_class_test_class*>( luaL_checkudata( l_, index_, name ) );
        if ( !self ) {
            luaL_typerror( l_, index_, name );
            throw std::runtime_error { "bad lua var" };
        }
        return *self;
    }

    static int method_router( ::lua_State* l_ ) {
        auto table = reinterpret_cast<const lua_CFunction*>( lua_touserdata( l_, lua_upvalueindex( 1 ) ) );
        int result = 0;
        for ( auto e = table; e; ++e ) {
            if ( ( result = ( *e )( l_ ) ) ) {
                break;
            }
        }
        return result - 1;
    }

    static void reg_type( ::lua_State* l_ ) {
        auto tables = begin_class_reg( l_ );
        auto metatable = tables._meta_table;
        auto methods = tables._method_table;
        auto geter = tables._geter_table;
        auto seter = tables._seter_table;

        add_method( l_, "print", methods, &register_class_test_class::print );
        add_method( l_, "set_name", methods, &register_class_test_class::set_name );
        add_method( l_, "get_name", methods, &register_class_test_class::get_name );
        add_method( l_, "set_name2", methods, &register_class_test_class::set_name2 );

        begin_overloaded_method( l_, "set" );
        add_overloaded_method_variant( l_, (void( register_class_test_class::*)(int)) &register_class_test_class::set );
        add_overloaded_method_variant( l_, (void( register_class_test_class::* )(const char*))&register_class_test_class::set );
        end_overloaded_method( l_, methods );

        add_property( l_, "my_name", geter, seter, &register_class_test_class::name_get, &register_class_test_class::name_set );
        add_member( l_, "_name", geter, seter, &register_class_test_class::_name );

        add_constant( l_, "default", methods, "default" );

        end_class_reg( l_, tables );
    }
};

const char* type_trait<register_class_test_class>::name = "test";
}

Test( context, register_class ) {
    lua::unique_context ctx;
    ctx.open( {} );
    gctx = &__tctx__;

    ctx.reg_type<register_class_test_class>();
    auto test_code = "function test:print_test() self:print() end\nlocal t = test(\"first\")\nt:print_test()\nt:set_name(\"second\")\nt:print()\nt:set_name(t:get_name()..5)\nt:print()\nt:set_name2(\"this\",5,\"is ok\")\nt:print()\nt:set(5)\nt:print()\nt:set(\"test\")\nt:print()\nt.my_name = \"my name\"\nt:print()\nt.my_name = t.my_name .. 5\nt:print()\nt._name = \"_name\"\nt:print()\n t._name = test.default\nt:print()\n";
    bool ok = ( 0 == luaL_dostring( ctx.get(), test_code ) );
    if ( !ok ) {
        auto error = ctx.to_string( -1 );
        if ( error ) {
            INFO_LOG( "lua error was %s", error );
        }
    }
    EXPECT_TRUE( ok, "function test:print_test() self:print() end\nlocal t = test(\"first\")\nt:print_test()\nt:set_name(\"second\")\nt:print()\nt:set_name(t:get_name()..5)\nt:print()\nt:set_name2(\"this\",5,\"is ok\")\nt:print()\nt:set(5)\nt:print()\nt:set(\"test\")\nt:print()\nt.my_name = \"my name\"\nt:print()\nt.my_name = t.my_name .. 5\nt:print()\nt._name = \"_name\"\nt:print()\n t._name = test.default\nt:print()\n" );
}

Test( context, function ) {
    lua::unique_context ctx;
    ctx.open( {} );
    gctx = &__tctx__;

    bool exec = false;
    ctx.push( std::function<void( const char*msg )>( [&__tctx__,&exec]( const char* str_ ) {
        __tctx__.info_log( "function %s", str_ );
        exec = !strcmp( str_, "test message" );
    } ) );
    ctx.set_global( "test_func" );
    auto test_code = "test_func(\"test message\")";
    bool ok = ( 0 == luaL_dostring( ctx.get(), test_code ) );
    if ( !ok ) {
        auto error = ctx.to_string( -1 );
        if ( error ) {
            INFO_LOG( "lua error was %s", error );
        }
    }
    EXPECT_TRUE( ok, "test_func(\"test message\")" );
    EXPECT_TRUE( exec, "test_func invoked" );
}

void main() {
    do_all_tests();
}