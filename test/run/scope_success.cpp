/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file   scope_success.cpp
 * \author Andrey Semashev
 *
 * \brief  This file contains tests for \c scope_success.
 */

#include <boost/scope/scope_success.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/lightweight_test_trait.hpp>
#include <boost/config.hpp>
#include <utility>
#include <stdexcept>
#include "function_types.hpp"

int g_n = 0;

void check_normal()
{
    int n = 0;
    {
        boost::scope::scope_success< normal_func > guard{ normal_func(n) };
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_success< normal_func > guard{ normal_func(n) };
        BOOST_TEST(guard.active());
        guard.release();
        BOOST_TEST(!guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_success< moveable_only_func > guard{ moveable_only_func(n) };
        BOOST_TEST(guard.active());
        guard.set_active(false);
        BOOST_TEST(!guard.active());
        guard.set_active(true);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_success< normal_func > guard(normal_func(n), false);
        BOOST_TEST(!guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_success< normal_func > guard(normal_func(n), false);
        BOOST_TEST(!guard.active());
        guard.set_active(true);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_success< moveable_only_func > guard1{ moveable_only_func(n) };
        BOOST_TEST(guard1.active());
        boost::scope::scope_success< moveable_only_func > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_success< moveable_only_func > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        normal_func func(n);
        boost::scope::scope_success< normal_func& > guard(func);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        normal_func func(n);
        boost::scope::scope_success< normal_func& > guard1(func);
        BOOST_TEST(guard1.active());
        boost::scope::scope_success< normal_func& > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_success< normal_func& > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(n, 1);

    struct local
    {
        static void raw_func()
        {
            ++g_n;
        }
    };

    g_n = 0;
    {
        boost::scope::scope_success< void (&)() > guard(local::raw_func);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(g_n, 1);

    g_n = 0;
    {
        boost::scope::scope_success< void (&)() > guard1(local::raw_func);
        BOOST_TEST(guard1.active());
        boost::scope::scope_success< void (&)() > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_success< void (&)() > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(g_n, 1);
}

void check_throw()
{
    int n = 0;
    try
    {
        boost::scope::scope_success< normal_func > guard{ normal_func(n) };
        BOOST_TEST(guard.active());
        throw std::runtime_error("error");
    }
    catch (...) {}
    BOOST_TEST_EQ(n, 0);

    n = 0;
    try
    {
        boost::scope::scope_success< normal_func > guard1{ normal_func(n) };
        BOOST_TEST(guard1.active());
        throw std::runtime_error("error");
    }
    catch (...)
    {
        BOOST_TEST_EQ(n, 0);

        boost::scope::scope_success< normal_func > guard2{ normal_func(n) };
        BOOST_TEST(guard2.active());
    }
    BOOST_TEST_EQ(n, 1);

#if !defined(BOOST_CORE_UNCAUGHT_EXCEPTIONS_EMULATED)
    n = 0;
    try
    {
        boost::scope::scope_success< normal_func > guard1{ normal_func(n) };
        BOOST_TEST(guard1.active());
        throw std::runtime_error("error 1");
    }
    catch (...)
    {
        BOOST_TEST_EQ(n, 0);

        try
        {
            boost::scope::scope_success< normal_func > guard2{ normal_func(n) };
            BOOST_TEST(guard2.active());
            throw std::runtime_error("error 2");
        }
        catch (...) {}
    }
    BOOST_TEST_EQ(n, 0);
#endif // !defined(BOOST_CORE_UNCAUGHT_EXCEPTIONS_EMULATED)

    n = 0;
    try
    {
        boost::scope::scope_success< throw_on_copy_func > guard{ throw_on_copy_func(n) };
        BOOST_ERROR("An exception is expected to be thrown by throw_on_copy_func");
    }
    catch (...) {}
    BOOST_TEST_EQ(n, 0);

    n = 0;
    try
    {
        boost::scope::scope_success< throw_on_move_func > guard{ throw_on_move_func(n) };
    }
    catch (...)
    {
        BOOST_ERROR("An exception is not expected to be thrown by throw_on_move_func (copy ctor should be used)");
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    bool scope_ended = false, exception_thrown = false, func_destroyed = false;
    try
    {
        boost::scope::scope_success< throw_on_call_func > guard{ throw_on_call_func(n, func_destroyed) };
        func_destroyed = false;
        scope_ended = true;
    }
    catch (...)
    {
        exception_thrown = true;
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST(scope_ended);
    BOOST_TEST(exception_thrown);
    BOOST_TEST(func_destroyed);
}

void check_deduction()
{
    int n = 0;
    {
        auto guard = boost::scope::make_scope_success(normal_func(n));
        BOOST_TEST(guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_success< normal_func >);
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        auto guard = boost::scope::make_scope_success(normal_func(n), false);
        BOOST_TEST(!guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_success< normal_func >);
    }
    BOOST_TEST_EQ(n, 0);

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
    n = 0;
    {
        boost::scope::scope_success guard{ normal_func(n) };
        BOOST_TEST(guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_success< normal_func >);
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_success guard([&n] { ++n; });
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_success guard1{ normal_func(n) };
        boost::scope::scope_success guard2 = std::move(guard1);
        BOOST_TEST(guard2.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard2), boost::scope::scope_success< normal_func >);
    }
    BOOST_TEST_EQ(n, 1);
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
}

int main()
{
    check_normal();
    check_throw();
    check_deduction();

    return boost::report_errors();
}
