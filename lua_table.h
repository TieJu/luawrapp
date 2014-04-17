#pragma once

extern "C" {
#include <lua.h>
}

#include <utility>

namespace lua {
template<typename TableRef, typename IndexRef>
class table_enty {
    TableRef    _table;
    IndexRef    _table;

public:

    template<typename Type>
    void set( Type value_ ) {
        _table.state().push( std::forward<Type>( value_ ) );
    }
};
}