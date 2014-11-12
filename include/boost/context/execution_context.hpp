
//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_EXECUTION_CONTEXT_H
#define BOOST_CONTEXT_EXECUTION_CONTEXT_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>

#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/context/detail/config.hpp>
#include <boost/context/fcontext.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace context {

#if defined(BOOST_USE_SEGMENTED_STACKS)
struct stack_context
{
    typedef void *  segments_context[BOOST_COROUTINES_SEGMENTS];

    std::size_t             size;
    void                *   sp;
    segments_context        segments_ctx;

    stack_context() :
        size( 0), sp( 0), segments_ctx()
    {}
};
#else
struct stack_context
{
    std::size_t             size;
    void                *   sp;

    stack_context( std::size_t size_, void * sp_) :
        size( size_), sp( sp_)
    {}
};
#endif

class fixedsize {
private:
    std::size_t     size_;

public:
    fixedsize() :
        size_( 64*1024) {
    }

    fixedsize( std::size_t size) :
        size_( size) {
    }

    ~fixedsize() {
    }

    stack_context allocate() {
        void * vp = std::malloc( size_);
        if ( nullptr == vp) throw std::bad_alloc();
        return stack_context( size_, static_cast< char * >( vp) + size_);
    }

    void deallocate( stack_context & sctx) {
        void * vp = static_cast< char * >( sctx.sp) - sctx.size;
        std::free( vp);
    }
};

class segmented {
private:
    std::size_t     size_;

public:
    segmented() :
        size_( 64*1024) {
    }

    segmented( std::size_t size) :
        size_( size) {
    }

    ~segmented() {
    }
};

class execution_context {
private:
    struct base_context {
        std::size_t     use_count;
        fcontext_t      fctx;

        base_context() : use_count( 0), fctx( 0) {} 

        base_context( fcontext_t fctx_) : use_count( 0), fctx( fctx_) {} 

        ~base_context() {}

        virtual void run() = 0;
        virtual void deallocate() = 0;

        void jump_to( base_context & from) {
            jump_fcontext( & from.fctx, fctx, reinterpret_cast< intptr_t >( this) );
        }

        friend void intrusive_ptr_add_ref( base_context * ctx)
        { ++ctx->use_count; }

        friend void intrusive_ptr_release( base_context * ctx)
        {
            BOOST_ASSERT( nullptr != ctx);

            if ( 0 == --ctx->use_count)
            {
                ctx->deallocate();
            }
        }
    };

    template< typename Fn, typename StackAlloc >
    struct side_context : public base_context {
        void    *   vp;
        std::size_t size;
        Fn          fn;
        StackAlloc  salloc;

        side_context( void * vp_, std::size_t size_, Fn && fn_, fcontext_t fctx, StackAlloc const& salloc_) :
            base_context( fctx),
            vp( vp_),
            size( size_),
            fn( std::forward< Fn >( fn_) ),
        salloc( salloc_) {
        }

        void deallocate() {
            std::free( vp);
        }

        void run() {
            try {
                fn();
            } catch (...) {
                std::terminate();
            }
        }
    };

    struct main_context : public base_context {
        void deallocate() {}
        void run() {}
    };

    static void entry_func( intptr_t p) noexcept {
        assert( 0 != p);

        void * vp( reinterpret_cast< void * >( p) );
        assert( nullptr != vp);

        base_context * bp( static_cast< base_context * >( vp) );
        assert( nullptr != bp);

        bp->run();
    }

    typedef boost::intrusive_ptr< base_context >    ptr_t;

    static thread_local ptr_t                       current_ctx_;

    boost::intrusive_ptr< base_context >            ptr_;

public:
    execution_context() :
        ptr_( current_ctx_) {
    }

    template< typename Fn >
    execution_context( Fn && fn, fixedsize salloc) :
        ptr_() {
        typedef side_context< Fn, fixedsize >  func_t;

        stack_context sctx( salloc.allocate() );
        std::size_t ssize = sctx.size - sizeof( func_t);
        void * vp = sctx.sp;
        void * sp = static_cast< char * >( vp) + ssize;
        ptr_.reset(
            new ( sp) func_t( vp, size,
                              std::forward< Fn >( fn),
                              make_fcontext( sp, ssize, & execution_context::entry_func) ) );
    }

    template< typename Fn >
    execution_context( Fn && fn, segmented stack) :
        ptr_() {
        typedef side_context< Fn, segmented >  func_t;

        std::size_t size = 64 * 1024;
        std::size_t ssize = size - sizeof( func_t);
        void * vp = std::malloc( size);
        void * sp = static_cast< char * >( vp) + ssize;
        ptr_.reset(
            new ( sp) func_t( vp, size,
                              std::forward< Fn >( fn),
                              make_fcontext( sp, ssize, & execution_context::entry_func) ) );
    }

    void jump_to() {
        assert( * this);
        ptr_t tmp( current_ctx_);
        current_ctx_ = ptr_;
        ptr_->jump_to( * tmp);
    }

    explicit operator bool() const noexcept {
        return nullptr != ptr_;
    }

    bool operator!() const noexcept {
        return nullptr == ptr_;
    }
};

thread_local
execution_context::ptr_t
execution_context::current_ctx_ = new execution_context::main_context();

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_CONTEXT_EXECUTION_CONTEXT_H

