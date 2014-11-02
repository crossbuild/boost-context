
//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_EXECUTION_CONTEXT_H
#define BOOST_CONTEXT_EXECUTION_CONTEXT_H

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility/explicit_operator_bool.hpp>

#include <boost/context/detail/config.hpp>
#include <boost/context/fconfig.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace context {

class execution_context {
private:
    fcontext_t      ctx_;

public:
    template< typename Fn >
    execution_context( BOOST_RV_REF( Fn) fn) :
        ctx_( 0)
    {}

    void jump_to( execution_context &);

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == ctx_; }
}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_CONTEXT_EXECUTION_CONTEXT_H

