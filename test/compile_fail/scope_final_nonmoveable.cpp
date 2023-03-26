/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file   scope_final_nonmoveable.cpp
 * \author Andrey Semashev
 *
 * \brief  This file tests that \c scope_final is nonmoveable.
 */

#include <boost/scope/scope_final.hpp>
#include <utility>
#include "function_types.hpp"

int main()
{
    int n = 0;
    boost::scope::scope_final< normal_func > guard1{ normal_func(n) };
    boost::scope::scope_final< normal_func > guard2 = std::move(guard1);

    return 0;
}
