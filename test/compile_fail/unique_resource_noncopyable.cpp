/*
 *             Copyright Andrey Semashev 2023.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   unique_resource_noncopyable.cpp
 * \author Andrey Semashev
 *
 * \brief  This file tests that \c unique_resource is noncopyable.
 */

#include <boost/scope/unique_resource.hpp>

template< typename Resource >
struct empty_resource_deleter
{
    void operator() (Resource const& res) const noexcept
    {
    }
};

int main()
{
    boost::scope::unique_resource< int, empty_resource_deleter< int > > ur1{ 10, empty_resource_deleter< int >() };
    boost::scope::unique_resource< int, empty_resource_deleter< int > > ur2 = ur1;

    return 0;
}
