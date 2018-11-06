#ifndef SIMPLESYMBOLENGINE_H
#define SIMPLESYMBOLENGINE_H

/**@file

   Wrapper for DbgHelper to provide common utility functions for processing
   Microsoft PDB information.

    @author Roger Orr <rogero@howzatt.demon.co.uk>

    Copyright &copy; 2004.
    This software is distributed in the hope that it will be useful, but
    without WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission is granted to anyone to make or distribute verbatim
    copies of this software provided that the copyright notice and
    this permission notice are preserved, and that the distributor
    grants the recipent permission for further distribution as permitted
    by this notice.

    Comments and suggestions are always welcome.
    Please report bugs to rogero@howzatt.demon.co.uk.

    $Revision: 1.2 $
*/

// $Id: SimpleSymbolEngine.h,v 1.2 2005/05/02 22:25:41 Administrator Exp $

#include <iosfwd>
#include <string>
#include <windows.h>
#include <dbghelp.h>

/** Symbol Engine wrapper to assist with processing PDB information */
class SimpleSymbolEngine
{
public:
    /** Get the symbol engine for this process */
    static SimpleSymbolEngine &instance();

    /** Convert an address to a string */
    std::string addressToString( PVOID address );

    /** Provide a stack trace for the specified stack frame */
    void StackTrace( PCONTEXT pContext, std::ostream & os );

private:
	// 显示的声明类拷贝的构造函数，和重载 = 操作符, 声明成私有的，并且只声明不实现。
	// 此时，SimpleSymbolEngine singleton = SimpleSymbolEngine::instance();　不管是在友元类中还是其他的，编译器都是报错
    /* don't copy or assign */
    SimpleSymbolEngine( SimpleSymbolEngine const & );
    SimpleSymbolEngine& operator=( SimpleSymbolEngine const & );

    /* Construct wrapper for this process */
    SimpleSymbolEngine();

    HANDLE hProcess;

public: // Work around for MSVC 6 bug
    /* Destroy information for this process */
    ~SimpleSymbolEngine();
private:
};

#endif // SIMPLESYMBOLENGINE_H
