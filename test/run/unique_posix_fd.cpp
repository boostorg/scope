/*
 *             Copyright Andrey Semashev 2023.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   unique_posix_fd.cpp
 * \author Andrey Semashev
 *
 * \brief  This file contains tests for \c unique_posix_fd.
 */

#include <boost/config.hpp>

#if defined(BOOST_HAS_UNISTD_H)

#include <boost/scope/unique_posix_fd.hpp>
#include <boost/core/lightweight_test.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

int main()
{
    {
        boost::scope::unique_posix_fd ur;
        BOOST_TEST_LT(ur.get(), 0);
        BOOST_TEST(!ur.allocated());
    }

    int fd = 0;
    {
        boost::scope::unique_posix_fd ur(::open("/dev/null", O_RDONLY));
        BOOST_TEST_GE(ur.get(), 0);
        BOOST_TEST(ur.allocated());
        fd = ur.get();
    }

    struct stat st = {};
    int res = ::fstat(fd, &st);
    BOOST_TEST_LT(res, 0);
    int err = errno;
    BOOST_TEST_EQ(err, EBADF);

    return boost::report_errors();
}

#else // !defined(BOOST_HAS_UNISTD_H)

int main()
{
    // This test is not supported on non-POSIX systems
    return 0;
}

#endif // !defined(BOOST_HAS_UNISTD_H)
