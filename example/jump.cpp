
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <boost/assert.hpp>
#include <boost/context/all.hpp>

#include "simple_stack_allocator.hpp"

namespace ctx = boost::context;

typedef ctx::simple_stack_allocator<
    8 * 1024 * 1024, // 8MB
    64 * 1024, // 64kB
    8 * 1024 // 8kB
>       stack_allocator;

void f1( ctx::transfer_t t1) {
    BOOST_ASSERT( 3 == (int)t1.data);
    std::cout << "f1: entered first time : " << (int)t1.data << std::endl;
    ctx::transfer_t t2 = ctx::jump_fcontext( t1.fctx, (void*)5);
    BOOST_ASSERT( 7 == (int)t2.data);
    std::cout << "f1: entered second time : " << (int)t2.data << std::endl;
    ctx::transfer_t t3 = ctx::jump_fcontext( t2.fctx, (void*)9);
    BOOST_ASSERT( 11 == (int)t3.data);
    std::cout << "f1: entered third time : " << (int)t3.data << std::endl;
    ctx::jump_fcontext( t3.fctx, (void*)13);
}

int main( int argc, char * argv[]) {
    stack_allocator alloc;

    void * base1 = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( base1, stack_allocator::default_stacksize(), f1);

    std::cout << "main: call start_fcontext( ctx, 0)" << std::endl;
    ctx::transfer_t t1 = ctx::jump_fcontext( ctx, (void*)3);
    BOOST_ASSERT( 5 == (int)t1.data);
    std::cout << "f1: returned first time : " << (int)t1.data << std::endl;
    ctx::transfer_t t2 = ctx::jump_fcontext( t1.fctx, (void*)7);
    BOOST_ASSERT( 9 == (int)t2.data);
    std::cout << "f1: returned second time : " << (int)t2.data << std::endl;
    ctx::transfer_t t3 = ctx::jump_fcontext( t2.fctx, (void*)11);
    BOOST_ASSERT( 13 == (int)t3.data);
    std::cout << "f1: returned third time : " << (int)t3.data << std::endl;

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
