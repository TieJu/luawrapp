#include "../include/luawrapp.h"

#include <utility>

#include <stdio.h>
#include <vector>
#include <type_traits>
#include <varargs.h>

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
    ctx.set_tabe( LUA_GLOBALSINDEX );
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), "print(\"test\", 8, 9)" ), "invokation of test_print_failed" );
}

Test( context, var_wrapper ) {
    lua::unique_context ctx;
    ctx.open( {} );

    gctx = &__tctx__;
    auto marker = ctx.mark_stack();
    auto global = ctx.get_global_var( "print" );
    global.set( test_print );
    EXPECT_TRUE( 0 == luaL_dostring( ctx.get(), "print(\"test\", 8, 9)" ), "invokation of test_print_failed" );
}
void main() {
    do_all_tests();
}