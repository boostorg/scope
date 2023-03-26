/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file   scope_check_noncopyable.cpp
 * \author Andrey Semashev
 *
 * \brief  This file tests that \c scope_check is noncopyable.
 */

#include <boost/scope/scope_check.hpp>
#include <boost/scope/error_code_checker.hpp>
#include "function_types.hpp"

int main()
{
    int n = 0, err = 0;
    boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard1{ normal_func(n), boost::scope::check_error_code(err) };
    boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard2 = guard1;

    return 0;
}
