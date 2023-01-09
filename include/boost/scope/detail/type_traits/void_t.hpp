/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/detail/type_traits/void_t.hpp
 *
 * This header contains definition of \c void_t type trait.
 */

#ifndef BOOST_SCOPE_DETAIL_TYPE_TRAITS_VOID_T_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_TYPE_TRAITS_VOID_T_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if (defined(__cpp_lib_void_t) && (__cpp_lib_void_t >= 201411l)) || \
    (defined(BOOST_MSSTL_VERSION) && BOOST_MSSTL_VERSION >= 140)

namespace boost {
namespace scope {
namespace detail {

using std::void_t;

} // namespace detail
} // namespace scope
} // namespace boost

#else

#include <boost/type_traits/make_void.hpp>

namespace boost {
namespace scope {
namespace detail {

using boost::void_t;

} // namespace detail
} // namespace scope
} // namespace boost

#endif

#endif // BOOST_SCOPE_DETAIL_TYPE_TRAITS_VOID_T_HPP_INCLUDED_
