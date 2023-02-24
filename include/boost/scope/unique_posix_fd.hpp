/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/unique_posix_fd.hpp
 *
 * This header contains definition of \c unique_posix_fd type.
 */

#ifndef BOOST_SCOPE_UNIQUE_POSIX_FD_HPP_INCLUDED_
#define BOOST_SCOPE_UNIQUE_POSIX_FD_HPP_INCLUDED_

#include <boost/scope/unique_resource.hpp>
#include <boost/scope/posix_fd_resource.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

//! Unique POSIX file descriptor resource
typedef unique_resource< int, posix_fd_deleter, posix_fd_resource_traits > unique_posix_fd;

} // namespace scope
} // namespace boost

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_UNIQUE_POSIX_FD_HPP_INCLUDED_
