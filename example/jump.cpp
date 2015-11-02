
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
    std::cout << "f1: entered first time" << std::endl;
    ctx::transfer_t t2 = ctx::jump_fcontext( t1.fctx, t1.data);
    std::cout << "f1: entered second time" << std::endl;
    ctx::transfer_t t3 = ctx::jump_fcontext( t2.fctx, t2.data);
    std::cout << "f1: entered third time" << std::endl;
    ctx::jump_fcontext( t3.fctx, t3.data);
}

int main( int argc, char * argv[]) {
    stack_allocator alloc;

    void * base1 = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( base1, stack_allocator::default_stacksize(), f1);

    std::cout << "main: call start_fcontext( ctx, 0)" << std::endl;
    ctx::transfer_t t1 = ctx::jump_fcontext( ctx, 0);
    std::cout << "f1: returned first time" << std::endl;
    ctx::transfer_t t2 = ctx::jump_fcontext( t1.fctx, 0);
    std::cout << "f1: returned second time" << std::endl;
    ctx::transfer_t t3 = ctx::jump_fcontext( t2.fctx, 0);
    std::cout << "f1: returned third time" << std::endl;

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
