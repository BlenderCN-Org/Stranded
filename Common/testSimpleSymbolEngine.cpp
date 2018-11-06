#include "SimpleSymbolEngine.h"

#include <string>
#include <iostream>

void fred( )
{
    std::string name( "fred" );

    CONTEXT context = {CONTEXT_FULL};
    ::GetThreadContext( GetCurrentThread(), &context );
    _asm call $+5
    _asm pop eax
    _asm mov context.Eip, eax
    _asm mov eax, esp
    _asm mov context.Esp, eax
    _asm mov context.Ebp, ebp

    SimpleSymbolEngine::instance().StackTrace( &context, std::cout );
}

void middle()
{
    std::string name( "middle" );

    fred();
}


void top()
{
    std::string name( "top" );

    middle();
}

int main()
{
    top();
    return 0;
}