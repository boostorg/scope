/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/scope_exit.hpp
 *
 * This header contains definition of \c scope_exit template.
 */

#ifndef BOOST_SCOPE_SCOPE_EXIT_HPP_INCLUDED_
#define BOOST_SCOPE_SCOPE_EXIT_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>
#include <boost/scope/detail/is_not_like.hpp>
#include <boost/scope/detail/compact_storage.hpp>
#include <boost/scope/detail/move_or_copy_ref.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

/*!
 * \brief Scope exit guard that invokes a function upon leaving the scope.
 *
 * A scope exit guard wraps a function object callable with no arguments
 * that can be one of:
 *
 * \li A user-defined class with a public <tt>operator()</tt>.
 * \li An lvalue reference to such class.
 * \li An lvalue reference to function taking no arguments.
 *
 * The scope guard can be in either active or inactive state. By default,
 * the constructed scope guard is active. When active, the scope guard
 * invokes the wrapped function object on destruction. If inactive,
 * the scope guard does not call the wrapped function object.
 *
 * The scope guard can be made inactive by moving-from the scope guard
 * or calling <tt>set_active(false)</tt> or <tt>release()</tt>. An inactive
 * scope guard can be made active by calling <tt>set_active(true)</tt>.
 * If a moved-from scope guard is active on destruction, the behavior
 * is undefined.
 */
template< typename Func >
class scope_exit
{
private:
    struct data :
        public detail::compact_storage< Func >
    {
        typedef detail::compact_storage< Func > func_base;

        bool m_active;

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, bool active) noexcept(std::is_nothrow_constructible< Func, F >::value) :
            func_base(static_cast< F&& >(func)),
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
            std::is_constructible< data, typename detail::move_or_copy_ref< F, Func >::type, bool >,
            detail::is_not_like< F, scope_exit >
        >::value >::type
    >
    explicit scope_exit(F&& func, bool active = true)
        noexcept(std::is_nothrow_constructible< data, typename detail::move_or_copy_ref< F, Func >::type, bool >::value) try :
        m_data(static_cast< typename detail::move_or_copy_ref< F, Func >::type >(func), active)
    {
    }
    catch (...)
    {
        if (active)
            func();
    }

    //! Move-constructs a scope guard, deactivates the original scope guard.
    template<
        bool Requires = std::is_constructible< data, typename detail::move_or_copy_ref< Func >::type, bool >::value,
        typename = typename std::enable_if< Requires >::type
    >
    scope_exit(scope_exit&& that) noexcept(std::is_nothrow_constructible< data, typename detail::move_or_copy_ref< Func >::type, bool >::value) :
        m_data(static_cast< typename detail::move_or_copy_ref< Func >::type >(that.m_data.func_base::get()), that.m_data.m_active)
    {
        that.m_data.m_active = false;
    }

    scope_exit& operator= (scope_exit&&) = delete;

    scope_exit(scope_exit const&) = delete;
    scope_exit& operator= (scope_exit const&) = delete;

    //! If active, invokes the wrapped callable function object. Destroys the callable.
    ~scope_exit() noexcept(noexcept(std::declval< Func& >()()))
    {
        if (BOOST_LIKELY(m_data.m_active))
            m_data.func_base::get()();
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
scope_exit(Func) -> scope_exit< Func >;

template< typename Func >
scope_exit(scope_exit< Func >&&) -> scope_exit< Func >;
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)

//! Creates a scope exit guard with a given callable function object.
template< typename Func >
inline scope_exit< Func > make_scope_exit(Func&& func, bool active = true)
    noexcept(std::is_nothrow_constructible< scope_exit< Func >, Func, bool >::value)
{
    return scope_exit< Func >(static_cast< Func&& >(func), active);
}

} // namespace scope

using scope::scope_exit;
using scope::make_scope_exit;

} // namespace boost

#endif // BOOST_SCOPE_SCOPE_EXIT_HPP_INCLUDED_
