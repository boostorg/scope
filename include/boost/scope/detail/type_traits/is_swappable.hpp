/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/detail/type_traits/is_swappable.hpp
 *
 * This header contains definition of \c is_swappable type trait.
 */

#ifndef BOOST_SCOPE_DETAIL_TYPE_TRAITS_IS_SWAPPABLE_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_TYPE_TRAITS_IS_SWAPPABLE_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if (defined(__cpp_lib_is_swappable) && (__cpp_lib_is_swappable >= 201603l)) || \
    (defined(BOOST_MSSTL_VERSION) && (BOOST_MSSTL_VERSION >= 140) && (_MSC_FULL_VER >= 190024210) && (BOOST_CXX_VERSION >= 201703l))

namespace boost {
namespace scope {
namespace detail {

using std::is_swappable;

} // namespace detail
} // namespace scope
} // namespace boost

#else

#include <utility>
#include <boost/scope/detail/type_traits/void_t.hpp>

namespace boost {
namespace scope {
namespace detail {
namespace is_swappable_detail {

using namespace std;

template< typename T, typename = void >
struct is_swappable_impl : public std::false_type { };
template< typename T >
struct is_swappable_impl< T, detail::void_t< decltype(swap(std::declval< T& >(), std::declval< T& >())) > > :
    public std::true_type
{
};

} // namespace is_swappable_detail

template< typename T >
using is_swappable = is_swappable_detail::is_swappable_impl< T >;

} // namespace detail
} // namespace scope
} // namespace boost

#endif

#endif // BOOST_SCOPE_DETAIL_TYPE_TRAITS_IS_SWAPPABLE_HPP_INCLUDED_
