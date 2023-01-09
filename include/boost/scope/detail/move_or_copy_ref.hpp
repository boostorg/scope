/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/detail/move_or_copy_ref.hpp
 *
 * This header contains definition of \c move_or_copy_ref type trait.
 */

#ifndef BOOST_SCOPE_DETAIL_MOVE_OR_COPY_REF_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_MOVE_OR_COPY_REF_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {
namespace detail {

//! The type trait produces an rvalue reference to \a From if \a To has a non-throwing constructor from a \a From rvalue and an lvalue reference otherwise.
template< typename From, typename To = From >
struct move_or_copy_ref
{
    struct from_rv
    {
        typedef From&& type;
    };

    typedef typename std::conditional<
        std::is_nothrow_constructible< To, From >::value,
        From&&,
        From const&
    >::type type;
};

template< typename From, typename To >
struct move_or_copy_ref< From&, To >
{
    typedef From& type;
};

} // namespace detail
} // namespace scope
} // namespace boost

#endif // BOOST_SCOPE_DETAIL_MOVE_OR_COPY_REF_HPP_INCLUDED_
