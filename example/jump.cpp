
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

void f1( ctx::transfer_t t_) {
    BOOST_ASSERT( 3 == (int)t_.data);
    std::cout << "f1: entered first time" << std::endl;
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, (void*)5);
    BOOST_ASSERT( 7 == (int)t.data);
    std::cout << "f1: entered second time" << std::endl;
    t = ctx::jump_fcontext( t.fctx, (void*)9);
    BOOST_ASSERT( 11 == (int)t.data);
    std::cout << "f1: entered third time" << std::endl;
    ctx::jump_fcontext( t.fctx, (void*)13);
}

int main( int argc, char * argv[]) {
    stack_allocator alloc;

    void * base1 = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( base1, stack_allocator::default_stacksize(), f1);

    std::cout << "main: call start_fcontext( ctx, 0)" << std::endl;
    ctx::transfer_t t = ctx::jump_fcontext( ctx, (void*)3);
    BOOST_ASSERT( 5 == (int)t.data);
    t = ctx::jump_fcontext( t.fctx, (void*)7);
    BOOST_ASSERT( 9 == (int)t.data);
    t = ctx::jump_fcontext( t.fctx, (void*)11);
    BOOST_ASSERT( 13 == (int)t.data);

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
