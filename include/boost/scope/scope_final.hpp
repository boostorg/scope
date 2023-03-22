/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/scope_final.hpp
 *
 * This header contains definition of \c scope_final template.
 */

#ifndef BOOST_SCOPE_SCOPE_FINAL_HPP_INCLUDED_
#define BOOST_SCOPE_SCOPE_FINAL_HPP_INCLUDED_

#include <type_traits>
#include <boost/scope/detail/config.hpp>
#include <boost/scope/detail/is_not_like.hpp>
#include <boost/scope/detail/move_or_copy_construct_ref.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

template< typename Func >
class scope_final;

namespace detail {

// Workaround for clang < 5.0 which can't pass scope_final as a template template parameter from within scope_final definition
template< typename T >
using is_not_like_scope_final = detail::is_not_like< T, scope_final >;

} // namespace detail

/*!
 * \brief Scope final guard that invokes a function upon leaving the scope.
 *
 * A scope exit guard wraps a function object callable with no arguments
 * that can be one of:
 *
 * \li A user-defined class with a public `operator()`.
 * \li An lvalue reference to such class.
 * \li An lvalue reference to function taking no arguments.
 *
 * The scope final guard unconditionally invokes the wrapped function object
 * on destruction.
 */
template< typename Func >
class scope_final
{
//! \cond
private:
    struct data
    {
        Func m_func;

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, std::true_type) noexcept :
            m_func(static_cast< F&& >(func))
        {
        }

        template< typename F, typename = typename std::enable_if< std::is_constructible< Func, F >::value >::type >
        explicit data(F&& func, std::false_type) try :
            m_func(static_cast< F&& >(func))
        {
        }
        catch (...)
        {
            func();
        }
    };

    data m_data;

//! \endcond
public:
    /*!
     * \brief Constructs a scope final guard with a given callable function object.
     *
     * **Requires:** \c Func is constructible from \a func.
     *
     * **Effects:** If \c Func is nothrow constructible from `F&&` then constructs \c Func from
     *              `std::forward< F >(func)`, otherwise constructs from `func`.
     *
     *              If \c Func construction throws, invokes \a func before returning with the exception.
     *
     * **Throws:** Nothing, unless construction of the function object throws.
     *
     * \param func The callable function object to invoke on destruction.
     */
    template<
        typename F
        //! \cond
        , typename = typename std::enable_if< detail::conjunction<
            std::is_constructible<
                data,
                typename detail::move_or_copy_construct_ref< F, Func >::type,
                typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< F, Func >::type >::type
            >,
            detail::is_not_like_scope_final< F >
        >::value >::type
        //! \endcond
    >
    scope_final(F&& func)
        noexcept(BOOST_SCOPE_DETAIL_DOC_HIDDEN(
            std::is_nothrow_constructible<
                data,
                typename detail::move_or_copy_construct_ref< F, Func >::type,
                typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< F, Func >::type >::type
            >::value
        )) :
        m_data
        (
            static_cast< typename detail::move_or_copy_construct_ref< F, Func >::type >(func),
            typename std::is_nothrow_constructible< Func, typename detail::move_or_copy_construct_ref< F, Func >::type >::type()
        )
    {
    }

    scope_final(scope_final const&) = delete;
    scope_final& operator= (scope_final const&) = delete;

    /*!
     * \brief Invokes the wrapped callable function object and destroys the callable.
     *
     * **Throws:** Nothing, unless invoking the callable throws.
     */
    ~scope_final() noexcept(noexcept(std::declval< Func& >()()))
    {
        m_data.m_func();
    }
};

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
template< typename Func >
scope_final(Func) -> scope_final< Func >;
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)

} // namespace scope

//! \cond
#if defined(BOOST_MSVC)
#define BOOST_SCOPE_DETAIL_UNIQUE_VAR_TAG __COUNTER__
#else
#define BOOST_SCOPE_DETAIL_UNIQUE_VAR_TAG __LINE__
#endif
//! \endcond

/*!
 * \brief The macro creates a uniquely named final scope exit guard.
 *
 * The macro should be followed by a function object that should be called
 * on leaving the current scope. Usage example:
 *
 * ```
 * BOOST_SCOPE_FINAL []
 * {
 *     std::cout << "Hello world!" << std::endl;
 * };
 * ```
 *
 * \note Using this macro requires C++17.
 */
#define BOOST_SCOPE_FINAL \
    boost::scope::scope_final BOOST_JOIN(_boost_scope_final_, BOOST_SCOPE_DETAIL_UNIQUE_VAR_TAG) =

} // namespace boost

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_SCOPE_FINAL_HPP_INCLUDED_
