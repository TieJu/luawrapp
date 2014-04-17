#pragma once


namespace lua {
inline int error_handler(lua_State *l_) {
    lua_getglobal( l_, "debug" );
    if ( !lua_istable( l_, -1 ) ) {
        lua_pop( l_, 1 );
        return 1;
    }

    lua_getfield( l_, -1, "traceback" );
    if ( !lua_isfunction( l_, -1 ) ) {
        lua_pop( l_, 2 );
        return 1;
    }
    lua_pushvalue( l_, 1 );
    lua_pushinteger( l_, 1 );
    lua_call( l_, 2, 1 );
    return 1;
}
}