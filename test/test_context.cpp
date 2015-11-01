
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>

#include <boost/context/all.hpp>
#include <boost/context/detail/config.hpp>

#include "../example/simple_stack_allocator.hpp"

namespace ctx = boost::context;

typedef ctx::simple_stack_allocator<
    8 * 1024 * 1024, // 8MB
    64 * 1024, // 64kB
    8 * 1024 // 8kB
>       stack_allocator;

int value1 = 0;
std::string value2;
double value3 = 0.;

void f1( ctx::transfer_t t) {
    ++value1;
    ctx::jump_fcontext( t.fctx, t.data);
}

void f3( ctx::transfer_t t_) {
    ++value1;
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, 0);
    ++value1;
    ctx::jump_fcontext( t.fctx, t.data);
}

void f4( ctx::transfer_t t) {
    int i = 7;
    ctx::jump_fcontext( t.fctx, & i);
}

void f5( ctx::transfer_t t) {
    ctx::jump_fcontext( t.fctx, t.data);
}

void f6( ctx::transfer_t t_) {
    std::pair< int, int > data = * ( std::pair< int, int > * ) t_.data;
    int res = data.first + data.second;
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, & res);
    data = * ( std::pair< int, int > *) t.data;
    res = data.first + data.second;
    ctx::jump_fcontext( t.fctx, & res);
}

void f7( ctx::transfer_t t) {
    try {
        throw std::runtime_error( * ( std::string *) t.data);
    } catch ( std::runtime_error const& e) {
        value2 = e.what();
    }
    ctx::jump_fcontext( t.fctx, t.data);
}

void f8( ctx::transfer_t t) {
    double d = * ( double *) t.data;
    d += 3.45;
    value3 = d;
    ctx::jump_fcontext( t.fctx, 0);
}

void f10( ctx::transfer_t t) {
    value1 = 3;
    ctx::jump_fcontext( t.fctx, 0);
}

void f9( ctx::transfer_t t) {
    std::cout << "f1: entered" << std::endl;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f10);
    ctx::jump_fcontext( ctx, 0);
    ctx::jump_fcontext( t.fctx, 0);
}

void test_setup() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f1);
    BOOST_CHECK( ctx);
}

void test_start() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f1);
    BOOST_CHECK( ctx);

    BOOST_CHECK_EQUAL( 0, value1);
    ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 1, value1);
}

void test_jump() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f3);
    BOOST_CHECK( ctx);
    BOOST_CHECK_EQUAL( 0, value1);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 1, value1);
    ctx::jump_fcontext( t.fctx, 0);
    BOOST_CHECK_EQUAL( 2, value1);
}

void test_result() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f4);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, 0);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 7, result);
}

void test_arg() {
    stack_allocator alloc;
    int i = 7;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f5);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, & i);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( i, result);
}

void test_transfer() {
    stack_allocator alloc;
    std::pair< int, int > data = std::make_pair( 3, 7);
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f6);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, & data);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 10, result);
    data = std::make_pair( 7, 7);
    t = ctx::jump_fcontext( t.fctx, & data);
    result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 14, result);
}

void test_exception() {
    stack_allocator alloc;
    std::string what("hello world");
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f7);
    BOOST_CHECK( ctx);
    ctx::jump_fcontext( ctx, & what);
    BOOST_CHECK_EQUAL( std::string( what), value2);
}

void test_fp() {
    stack_allocator alloc;
    double d = 7.13;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f8);
    BOOST_CHECK( ctx);
    ctx::jump_fcontext( ctx, & d);
    BOOST_CHECK_EQUAL( 10.58, value3);
}

void test_stacked() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f9);
    ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 3, value1);
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* []) {
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.Context: context test suite");

    test->add( BOOST_TEST_CASE( & test_setup) );

    test->add( BOOST_TEST_CASE( & test_start) );
    test->add( BOOST_TEST_CASE( & test_jump) );
    test->add( BOOST_TEST_CASE( & test_result) );
    test->add( BOOST_TEST_CASE( & test_arg) );
    test->add( BOOST_TEST_CASE( & test_transfer) );
    test->add( BOOST_TEST_CASE( & test_exception) );
    test->add( BOOST_TEST_CASE( & test_fp) );
    test->add( BOOST_TEST_CASE( & test_stacked) );

    return test;
}
