#pragma once

#include <functional>

namespace lua {
template<typename Derived>
class namespace_def {
public:
    template<typename Signature>
    void function( const char* name_, std::function<Signature> func_ );

    template<typename RetType, typename... Args>
    void function( const char* name_, RetType( *func_ )( Args... ) );

    template<typename Type>
    void property( const char* name_, std::function<Type()> geter_, std::function<void( Type )> seter_, bool writeable_ = true, bool readable_ = true );

    template<typename Type>
    void constant( const char* name_, Type value_ );
};
}