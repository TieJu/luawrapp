#pragma once

namespace lua {
template<typename State>
class stack {
    State   _state;
    int     _top;

public:
    explicit stack( State state_ ) : _state { std::move( state_ ) } { recapture_top(); }
    ~stack() { reset_top(); }

    void recapture_top() {
        _top = _state.get_top();
    }

    void reset_top() {
        _state.set_top( _top );
    }

    template<typename Type>
    void push( Type value_ ) {
        _state.push( std::forward<Type>( value_ ) );
    }

    void pop( int c_ ) {
        _state.pop( c_ );
    }

    template<typename Type>
    Type to( int index_ ) {
        return _state.to<Type>( index_ );
    }

    void push_value( int index_ ) {
        _state.push_value( index_ );
    }

    template<typename Type>
    bool is( int index_ ) {
        return _state.is( index_ );
    }

    void remove( int index_ ) {
        _state.remove( index_ );
    }

    void insert( int index_ ) {
        _state.insert( index_ );
    }
    
    void replace( int index_ ) {
        _state.replace( index_ );
    }

    void check_stack( int size_ ) {
        _state.check_stack( size_ );
    }

    int type( int index_ ) {
        return _state.type( index_ );
    }

    bool equal( int first_, int second_ ) {
        return _state.equal( first_, second_ );
    }

    bool raw_equal( int first_, int second_ ) {
        return _state.raw_equal( first_, second_ );
    }

    bool less_than( int first_, int second_ ) {
        return  _state.less_than( first_, second_ );
    }

    size_t obj_len( int index_ ) {
        return _state.obj_len( index_ );
    }

    void get_table( int index_ ) {
        _state.get_table( index_ );
    }

    void get_field( int index_, const char* str_ ) {
        _state.get_field( index_, str_ );
    }

    void raw_get( int index_ ) {
        _state.raw_get( index_ );
    }

    void raw_geti( int index_ ) {
        _state.raw_geti( index_ );
    }

    void create_table( int numbers_ = 0, int records_ = 0 ) {
        _state.create_table( numbers_, records_ );
    }

    void* new_user_data( size_t size_ ) {
        return _state.new_user_data( size_ );
    }

    bool get_meta_table( int index_ ) {
        return _state.get_meta_table( index_ );
    }

    void get_fenv( int index_ ) {
        _state.get_fenv( index_ );
    }

    void set_tabe( int index_ ) {
        _state.set_tabe( index_ );
    }

    void set_field( int index_, const char* str_ ) {
        _state.set_field( index_, str_ );
    }

    void raw_set( int index_ ) {
        _state.raw_set( index_ );
    }

    void raw_seti( int index_, int n_ ) {
        _state.raw_seti( index_, n_ );
    }

    void set_meta_table( int index_ ) {
        _state.set_meta_table( index_ );
    }

    void set_fenv( int index_ ) {
        _state.set_fenv( index_ );
    }

    void call( int num_args_, int num_results_ ) {
        _state.call( num_args_, num_results_ );
    }

    int pcall( int num_args_, int num_results_, int error_func_ ) {
        return _state.pcall( num_args_, num_results_, error_func_ );
    }

    int cpcall( lua_CFunction func_, void* ud_ ) {
        return _state.cpcall( func_, ud_ );
    }

    int next( int index_ ) {
        return _state.next( index_ );
    }

    void concat( int count_ ) {
        _state.concat( count_ );
    }

    size_t strlen( int index_ ) {
        return obj_len( index_ );
    }

    void new_table( int numbers_ = 0, int records_ = 0 ) {
        create_table( numbers_, records_ );
    }

    void set_global( const char* str_ ) {
        _state.set_global( str_ );
    }

    void get_global( const char* str_ ) {
        _state.get_global( str_ );
    }
};
}