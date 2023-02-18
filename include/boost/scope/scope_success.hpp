/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/scope_success.hpp
 *
 * This header contains definition of \c scope_success template.
 */

#ifndef BOOST_SCOPE_SCOPE_SUCCESS_HPP_INCLUDED_
#define BOOST_SCOPE_SCOPE_SUCCESS_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>
#include <boost/core/uncaught_exceptions.hpp>
#include <boost/scope/detail/is_not_like.hpp>
#include <boost/scope/detail/compact_storage.hpp>
#include <boost/scope/detail/move_or_copy_construct_ref.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

template< typename Func >
class scope_success;

namespace detail {

// Workaround for clang < 5.0 which can't pass scope_success as a template template parameter from within scope_success definition
template< typename T >
using is_not_like_scope_success = detail::is_not_like< T, scope_success >;

} // namespace detail

/*!
 * \brief Scope exit guard that invokes a function upon leaving the scope normally.
 *
 * A scope exit guard wraps a function object callable with no arguments
 * that can be one of:
 *
 * \li A user-defined class with a public <tt>operator()</tt>.
 * \li An lvalue reference to such class.
 * \li An lvalue reference to function taking no arguments.
 *
 * The scope guard can be in either active or inactive state. By default,
 * the constructed scope guard is active. When active, and the scope is
 * being left normally (not via an exception), the scope guard invokes
 * the wrapped function object on destruction. If inactive, the scope
 * guard does not call the wrapped function object.
 *
 * The scope guard can be made inactive by moving-from the scope guard
 * or calling <tt>set_active(false)</tt> or <tt>release()</tt>. An inactive
 * scope guard can be made active by calling <tt>set_active(true)</tt>.
 * If a moved-from scope guard is active on destruction, the behavior
 * is undefined.
 */
template< typename Func >
class scope_success
{
private:
    struct data :
        public detail::compact_storage< Func >
    {
        typedef detail::compact_storage< Func > func_base;

        const unsigned int m_uncaught_count;
        bool m_active;

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, unsigned int uncaught_count, bool active) noexcept(std::is_nothrow_constructible< Func, F >::value) :
            func_base(static_cast< F&& >(func)),
            m_uncaught_count(uncaught_count),
            m_active(active)
        {
        }
    };

    data m_data;

public:
    //! Constructs an active scope guard with a given callable function object.
    template<
        typename F,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< data, typename detail::move_or_copy_construct_ref< F, Func >::type, unsigned int, bool >,
            detail::is_not_like_scope_success< F >
        >::value >::type
    >
    explicit scope_success(F&& func, bool active = true)
        noexcept(std::is_nothrow_constructible< data, typename detail::move_or_copy_construct_ref< F, Func >::type, unsigned int, bool >::value) :
        m_data(static_cast< typename detail::move_or_copy_construct_ref< F, Func >::type >(func), boost::core::uncaught_exceptions(), active)
    {
    }

    //! Move-constructs a scope guard, deactivates the original scope guard.
    template<
        bool Requires = std::is_constructible< data, typename detail::move_or_copy_construct_ref< Func >::type, unsigned int, bool >::value,
        typename = typename std::enable_if< Requires >::type
    >
    scope_success(scope_success&& that) noexcept(std::is_nothrow_constructible< data, typename detail::move_or_copy_construct_ref< Func >::type, unsigned int, bool >::value) :
        m_data(static_cast< typename detail::move_or_copy_construct_ref< Func >::type >(that.m_data.get()), that.m_data.m_uncaught_count, that.m_data.m_active)
    {
        that.m_data.m_active = false;
    }

    scope_success& operator= (scope_success&&) = delete;

    scope_success(scope_success const&) = delete;
    scope_success& operator= (scope_success const&) = delete;

    //! If active, and the scope guard is not destroyed via an exception, invokes the wrapped callable function object. Destroys the callable.
    ~scope_success() noexcept(noexcept(std::declval< Func& >()()))
    {
        if (BOOST_LIKELY(m_data.m_active && boost::core::uncaught_exceptions() <= m_data.m_uncaught_count))
            m_data.get()();
    }

    //! Returns \c true if the scope guard is active, otherwise \c false.
    bool active() const noexcept
    {
        return m_data.m_active;
    }

    //! Activates or deactivates the scope guard.
    void set_active(bool active) noexcept
    {
        m_data.m_active = active;
    }

    //! Deactivates the scope guard.
    void release() noexcept
    {
        m_data.m_active = false;
    }
};

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
template< typename Func >
scope_success(Func) -> scope_success< Func >;

template< typename Func >
scope_success(scope_success< Func >&&) -> scope_success< Func >;
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)

//! Creates a scope success guard with a given callable function object.
template< typename Func >
inline scope_success< Func > make_scope_success(Func&& func, bool active = true)
    noexcept(std::is_nothrow_constructible< scope_success< Func >, Func, bool >::value)
{
    return scope_success< Func >(static_cast< Func&& >(func), active);
}

} // namespace scope

using scope::scope_success;
using scope::make_scope_success;

} // namespace boost

#endif // BOOST_SCOPE_SCOPE_SUCCESS_HPP_INCLUDED_
