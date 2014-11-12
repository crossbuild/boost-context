
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/assert.hpp>
#include <boost/context/execution_context.hpp>

boost::context::execution_context * ctx1 = 0;
boost::context::execution_context * ctx2 = 0;
boost::context::execution_context * ctx = 0;

void f1()
{
    std::cout << "f1: entered" << std::endl;
    ctx2->jump_to();
    std::cout << "f1: re-entered" << std::endl;
    ctx2->jump_to();
}

void f2()
{
    std::cout << "f2: entered" << std::endl;
    ctx1->jump_to();
    std::cout << "f2: re-entered" << std::endl;
    ctx->jump_to();
}

int main( int argc, char * argv[])
{
    boost::context::execution_context ctx1_( f1, boost::context::fixedsize() );
    ctx1 = & ctx1_;
    boost::context::execution_context ctx2_( f2, boost::context::fixedsize() );
    ctx2 = & ctx2_;
    boost::context::execution_context ctx_;
    ctx = & ctx_;
    
    ctx1->jump_to();

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
