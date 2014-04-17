#include "../include/luawrapp.h"

#include <utility>

#include <stdio.h>
#include <vector>
#include <type_traits>

#include <Windows.h>

struct test_context {
    HANDLE con;
    unsigned started { 0 };
    unsigned failed { 0 };
    unsigned passed { 0 };
    test_context() {
        con = GetStdHandle( STD_OUTPUT_HANDLE );
    }
    void begin_test( const char* name_ ) {
        SetConsoleTextAttribute( con, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED );
        printf( "test %s ...", name_ );
        ++started;
    }

    void test_ok() {
        SetConsoleTextAttribute( con, FOREGROUND_GREEN );
        printf( "%s\r\n", "ok" );
        ++passed;
    }

    void test_failed() {
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", "failed" );
        ++failed;
    }

    void test_failed( const char* msg_ ) {
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", "failed" );
        SetConsoleTextAttribute( con, FOREGROUND_RED );
        printf( "%s\r\n", msg_ );
        ++failed;
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
};
#define DefTest(name_, code_) struct name_ { name_(test_context& ctx_ ) { ctx_.begin_test(#name_); code_ ctx_.test_ok(); } };
DefTest( unique_context, {
    lua::unique_context ctx;
    ctx.open( {} );
} )
DefTest( shared_context, {
    lua::shared_context ctx;
    ctx.open( {} );

    auto copy = ctx;
    ctx.close();
} )

template<typename... Tests>
struct test_set {

    template<typename First>
    void do_tests( test_context& ctx_ ) {
        try {
            First f { ctx_ };
        } catch ( const std::exception& e_ ) {
            ctx_.test_failed( e_.what() );
        } catch ( ... ) {
            ctx_.test_failed();
        }
    }
    template<typename First, typename... Others>
    typename std::enable_if<sizeof...( Others ) != 0>::type do_tests( test_context & ctx_ ) {
        try {
            First f { ctx_ };
        } catch ( ... ) {
            ctx_.test_failed();
        }
        do_tests<Others...>(ctx_);
    }

    test_set() {
        test_context ctx;
        do_tests<Tests...>(ctx);
    }

}; 

void main() {
    test_set<unique_context, shared_context> test {};
}