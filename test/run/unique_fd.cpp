/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file   unique_fd.cpp
 * \author Andrey Semashev
 *
 * \brief  This file contains tests for \c unique_fd.
 */

#include <boost/config.hpp>

#include <boost/scope/unique_fd.hpp>
#include <boost/core/lightweight_test.hpp>

#if defined(BOOST_WINDOWS)
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>

#if defined(BOOST_WINDOWS)
#define open _open
#define O_RDONLY _O_RDONLY
#define stat _stat
#define fstat _fstat
#endif // defined(BOOST_WINDOWS)

int main(int argc, char* args[])
{
    {
        boost::scope::unique_fd ur;
        BOOST_TEST_LT(ur.get(), 0);
        BOOST_TEST(!ur.allocated());
    }

    if (argc > 0)
    {
        int fd = 0;
        {
            boost::scope::unique_fd ur(::open(args[0], O_RDONLY));
            BOOST_TEST_GE(ur.get(), 0);
            BOOST_TEST(ur.allocated());
            fd = ur.get();
        }

        struct stat st = {};
        int res = ::fstat(fd, &st);
        BOOST_TEST_LT(res, 0);
        int err = errno;
        BOOST_TEST_EQ(err, EBADF);
    }
    else
    {
        std::puts("Test executable file name not provided in process args");
    }

    return boost::report_errors();
}
