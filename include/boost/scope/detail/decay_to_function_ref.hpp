/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/detail/decay_to_function_ref.hpp
 *
 * This header contains definition of \c decay_to_function_ref type trait.
 */

#ifndef BOOST_SCOPE_DETAIL_DECAY_TO_FUNCTION_REF_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_DECAY_TO_FUNCTION_REF_HPP_INCLUDED_

#include <type_traits>
#include <boost/scope/detail/config.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {
namespace detail {

//! If \c T is a reference to function, the type trait produces it unchanged, otherwise it returns `std::remove_cvref_t< T >`.
template< typename T >
struct decay_to_function_ref
{
    typedef typename std::remove_cv< T >::type type;
};

template< typename T >
struct decay_to_function_ref< T& >
{
    typedef typename std::remove_cv< T >::type type;
};

template< typename T >
struct decay_to_function_ref< T&& >
{
    typedef typename std::remove_cv< T >::type type;
};

template< typename R, typename... Args >
struct decay_to_function_ref< R (&)(Args...) >
{
    typedef R (&type)(Args...);
};

#if !defined(BOOST_SCOPE_NO_CXX17_NOEXCEPT_FUNCTION_TYPES)

template< typename R, typename... Args >
struct decay_to_function_ref< R (&)(Args...) noexcept >
{
    typedef R (&type)(Args...) noexcept;
};

#endif // !defined(BOOST_SCOPE_NO_CXX17_NOEXCEPT_FUNCTION_TYPES)

} // namespace detail
} // namespace scope
} // namespace boost

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_DETAIL_DECAY_TO_FUNCTION_REF_HPP_INCLUDED_
