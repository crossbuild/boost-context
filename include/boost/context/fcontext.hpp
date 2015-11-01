
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_FCONTEXT_H
#define BOOST_CONTEXT_FCONTEXT_H

#if defined(__PGI)
#include <stdint.h>
#endif

#if defined(_WIN32_WCE)
typedef int intptr_t;
#endif

#include <boost/config.hpp>
#include <boost/cstdint.hpp>

#include <boost/context/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace context {

typedef void*   fcontext_t;

struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};

extern "C" BOOST_CONTEXT_DECL
transfer_t BOOST_CONTEXT_CALLDECL jump_fcontext( fcontext_t to, void * vp);
extern "C" BOOST_CONTEXT_DECL
fcontext_t BOOST_CONTEXT_CALLDECL make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_CONTEXT_FCONTEXT_H

