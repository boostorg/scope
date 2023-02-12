/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/posix_fd_resource.hpp
 *
 * This header contains definition of \c unique_resource utilities
 * for compatibility with POSIX file descriptors.
 */

#ifndef BOOST_SCOPE_POSIX_FD_RESOURCE_HPP_INCLUDED_
#define BOOST_SCOPE_POSIX_FD_RESOURCE_HPP_INCLUDED_

#include <boost/config.hpp>

#if !defined(BOOST_HAS_UNISTD_H)
#error "Boost.Scope: This header is only compatible with POSIX systems"
#endif

#include <unistd.h>
#if defined(hpux) || defined(_hpux) || defined(__hpux)
#include <cerrno>
#endif

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

//! POSIX file descriptor resource traits
struct posix_fd_resource_traits
{
    //! Creates a default fd value
    static int make_default() noexcept
    {
        return -1;
    }

    //! Tests if the fd is allocated (valid)
    static bool is_allocated(int fd) noexcept
    {
        return fd >= 0;
    }
};

//! POSIX file descriptor deleter
struct posix_fd_deleter
{
    typedef void result_type;

    result_type operator() (int fd) const noexcept
    {
        // Some systems don't close the file descriptor in case if the thread is interrupted by a signal and close(2) returns EINTR.
        // Other (most) systems do close the file descriptor even when when close(2) returns EINTR, and attempting to close it
        // again could close a different file descriptor that was opened by a different thread.
        //
        // Future POSIX standards will likely fix this by introducing posix_close (see https://www.austingroupbugs.net/view.php?id=529)
        // and prohibiting returning EINTR from close(2), but we still have to support older systems where this new behavior is not available and close(2)
        // behaves differently between systems.
#if defined(hpux) || defined(_hpux) || defined(__hpux)
        int res;
        while (true)
        {
            res = ::close(fd);
            if (BOOST_UNLIKELY(res < 0))
            {
                int err = errno;
                if (err == EINTR)
                    continue;
            }

            break;
        }
#else
        ::close(fd);
#endif
    }
};

} // namespace scope
} // namespace boost

#endif // BOOST_SCOPE_POSIX_FD_RESOURCE_HPP_INCLUDED_
