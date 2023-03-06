/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
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
#include <boost/scope/detail/config.hpp>
#include <boost/scope/detail/is_not_like.hpp>
#include <boost/scope/detail/compact_storage.hpp>
#include <boost/scope/detail/move_or_copy_construct_ref.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

template< typename Func >
class scope_exit;

namespace detail {

// Workaround for clang < 5.0 which can't pass scope_exit as a template template parameter from within scope_exit definition
template< typename T >
using is_not_like_scope_exit = detail::is_not_like< T, scope_exit >;

} // namespace detail

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
//! \cond
private:
    struct data :
        public detail::compact_storage< Func >
    {
        typedef detail::compact_storage< Func > func_base;

        bool m_active;

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, bool active, std::true_type) noexcept :
            func_base(static_cast< F&& >(func)),
            m_active(active)
        {
        }

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, bool active, std::false_type) try :
            func_base(static_cast< F&& >(func)),
            m_active(active)
        {
        }
        catch (...)
        {
            if (active)
                func();
        }
    };

    data m_data;

//! \endcond
public:
    /*!
     * \brief Constructs a scope guard with a given callable function object.
     *
     * <b>Requires:</b> \c Func is constructible from \a func.
     *
     * <b>Effects:</b> If \c Func is nothrow constructible from <tt>F&&</tt> then constructs \c Func from
     *                 <tt>std::forward< F >(func)</tt>, otherwise constructs from <tt>func</tt>.
     *
     *                 If \c Func construction throws and \a active is \c true, invokes \a func before
     *                 returning with the exception.
     *
     * <b>Throws:</b> Nothing, unless construction of the function object throws.
     *
     * \param func The callable function object to invoke on destruction.
     * \param active Indicates whether the scope guard should be active upon construction.
     *
     * \post <tt>this->active() == active</tt>
     */
    template<
        typename F
        //! \cond
        , typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< data, typename detail::move_or_copy_construct_ref< F, Func >::type, bool, typename std::is_nothrow_constructible< Func, F >::type >,
            detail::is_not_like_scope_exit< F >
        >::value >::type
        //! \endcond
    >
    explicit scope_exit(F&& func, bool active = true)
        noexcept(BOOST_SCOPE_DETAIL_DOC_HIDDEN(
            std::is_nothrow_constructible<
                data,
                typename detail::move_or_copy_construct_ref< F, Func >::type,
                bool,
                typename std::is_nothrow_constructible< Func, F >::type
            >::value
        )) :
        m_data(static_cast< typename detail::move_or_copy_construct_ref< F, Func >::type >(func), active, typename std::is_nothrow_constructible< Func, F >::type())
    {
    }

    /*!
     * \brief Move-constructs a scope guard.
     *
     * <b>Requires:</b> \c Func is nothrow move-constructible or copy-constructible.
     *
     * <b>Effects:</b> If \c Func is nothrow move-constructible then move-constructs \c Func from
     *                 a member of \a that, otherwise copy-constructs.
     *
     * <b>Throws:</b> Nothing, unless construction of the function object throws.
     *
     * \param that Move source.
     *
     * \post <tt>that.active() == false</tt>
     */
    //! \cond
    template<
        bool Requires = std::is_constructible<
            data,
            typename detail::move_or_copy_construct_ref< Func >::type,
            bool,
            typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< Func >::type >::type
        >::value,
        typename = typename std::enable_if< Requires >::type
    >
    //! \endcond
    scope_exit(scope_exit&& that)
        noexcept(BOOST_SCOPE_DETAIL_DOC_HIDDEN(
            std::is_nothrow_constructible<
                data,
                typename detail::move_or_copy_construct_ref< Func >::type,
                bool,
                typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< Func >::type >::type
            >::value
        )) :
        m_data
        (
            static_cast< typename detail::move_or_copy_construct_ref< Func >::type >(that.m_data.get()),
            that.m_data.m_active,
            typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< Func >::type >::type()
        )
    {
        that.m_data.m_active = false;
    }

    scope_exit& operator= (scope_exit&&) = delete;

    scope_exit(scope_exit const&) = delete;
    scope_exit& operator= (scope_exit const&) = delete;

    /*!
     * \brief If <tt>active() == true</tt>, invokes the wrapped callable function object. Destroys the callable.
     *
     * <b>Throws:</b> Nothing, unless invoking the callable throws.
     */
    ~scope_exit() noexcept(noexcept(std::declval< Func& >()()))
    {
        if (BOOST_LIKELY(m_data.m_active))
            m_data.get()();
    }

    /*!
     * \brief Returns \c true if the scope guard is active, otherwise \c false.
     *
     * <b>Throws:</b> Nothing.
     */
    bool active() const noexcept
    {
        return m_data.m_active;
    }

    /*!
     * \brief Activates or deactivates the scope guard.
     *
     * <b>Throws:</b> Nothing.
     *
     * \param active The active status to set.
     *
     * \post <tt>this->active() == active</tt>
     */
    void set_active(bool active) noexcept
    {
        m_data.m_active = active;
    }

    /*!
     * \brief Deactivates the scope guard.
     *
     * <b>Effects:</b> As if <tt>set_active(false)</tt>.
     *
     * <b>Throws:</b> Nothing.
     *
     * \post <tt>this->active() == false</tt>
     */
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

/*!
 * \brief Creates a scope exit guard with a given callable function object.
 *
 * <b>Effects:</b> Constructs a scope guard as if by calling
 *                 <tt>scope_exit< Func >(std::forward< Func >(func), active)</tt>.
 *
 * \param func The callable function object to invoke on destruction.
 * \param active Indicates whether the scope guard should be active upon construction.
 */
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

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_SCOPE_EXIT_HPP_INCLUDED_
