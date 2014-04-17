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
}

Test( context, shared ) {
    lua::shared_context ctx;
    ctx.open( {} );

    auto copy = ctx;

    ctx.close();
}
void main() {
    do_all_tests();
}