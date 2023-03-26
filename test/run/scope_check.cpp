/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file   scope_check.cpp
 * \author Andrey Semashev
 *
 * \brief  This file contains tests for \c scope_check.
 */

#include <boost/scope/scope_check.hpp>
#include <boost/scope/exception_checker.hpp>
#include <boost/scope/error_code_checker.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/lightweight_test_trait.hpp>
#include <boost/config.hpp>
#include <utility>
#include <stdexcept>
#include <system_error>
#include "function_types.hpp"

int g_n = 0, g_c = 0;

struct always_true
{
    bool operator()() const noexcept
    {
        return true;
    }
};

struct always_false
{
    bool operator()() const noexcept
    {
        return false;
    }
};

void check_normal()
{
    int n = 0;
    {
        boost::scope::scope_check< normal_func, always_true > guard{ normal_func(n), always_true() };
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_check< normal_func, always_false > guard{ normal_func(n), always_false() };
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_check< normal_func, always_true > guard{ normal_func(n) };
        BOOST_TEST(guard.active());
        guard.release();
        BOOST_TEST(!guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_check< moveable_only_func, always_true > guard{ moveable_only_func(n) };
        BOOST_TEST(guard.active());
        guard.set_active(false);
        BOOST_TEST(!guard.active());
        guard.set_active(true);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_check< normal_func, always_true > guard(normal_func(n), always_true(), false);
        BOOST_TEST(!guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_check< normal_func, always_true > guard(normal_func(n), false);
        BOOST_TEST(!guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        boost::scope::scope_check< normal_func, always_true > guard(normal_func(n), false);
        BOOST_TEST(!guard.active());
        guard.set_active(true);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_check< moveable_only_func, always_true > guard1{ moveable_only_func(n) };
        BOOST_TEST(guard1.active());
        boost::scope::scope_check< moveable_only_func, always_true > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_check< moveable_only_func, always_true > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        normal_func func(n);
        always_true cond;
        boost::scope::scope_check< normal_func&, always_true& > guard(func, cond);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        normal_func func(n);
        always_true cond;
        boost::scope::scope_check< normal_func&, always_true& > guard1(func, cond);
        BOOST_TEST(guard1.active());
        boost::scope::scope_check< normal_func&, always_true& > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_check< normal_func&, always_true& > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(n, 1);

    struct local
    {
        static void raw_func()
        {
            ++g_n;
        }

        static bool raw_cond()
        {
            ++g_c;
            return true;
        }
    };

    g_n = 0;
    g_c = 0;
    {
        boost::scope::scope_check< void (&)(), bool (&)() > guard(local::raw_func, local::raw_cond);
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(g_c, 1);
    BOOST_TEST_EQ(g_n, 1);

    g_n = 0;
    g_c = 0;
    {
        boost::scope::scope_check< void (&)(), bool (&)() > guard1(local::raw_func, local::raw_cond);
        BOOST_TEST(guard1.active());
        boost::scope::scope_check< void (&)(), bool (&)() > guard2 = std::move(guard1);
        BOOST_TEST(!guard1.active());
        BOOST_TEST(guard2.active());
        boost::scope::scope_check< void (&)(), bool (&)() > guard3 = std::move(guard1);
        BOOST_TEST(!guard3.active());
    }
    BOOST_TEST_EQ(g_c, 1);
    BOOST_TEST_EQ(g_n, 1);
}

void check_throw()
{
    int n = 0;
    try
    {
        boost::scope::scope_check< throw_on_copy_func, boost::scope::exception_checker > guard{ throw_on_copy_func(n) };
        BOOST_ERROR("An exception is expected to be thrown by throw_on_copy_func");
    }
    catch (...) {}
    BOOST_TEST_EQ(n, 1);

    n = 0;
    try
    {
        boost::scope::scope_check< throw_on_move_func, boost::scope::exception_checker > guard{ throw_on_move_func(n) };
    }
    catch (...)
    {
        BOOST_ERROR("An exception is not expected to be thrown by throw_on_move_func (copy ctor should be used)");
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    bool scope_ended = false, exception_thrown = false, func_destroyed = false;
    try
    {
        boost::scope::scope_check< throw_on_call_func, boost::scope::exception_checker > guard{ throw_on_call_func(n, func_destroyed) };
        func_destroyed = false;
        scope_ended = true;
    }
    catch (...)
    {
        exception_thrown = true;
    }
    BOOST_TEST_EQ(n, 0);
    BOOST_TEST(scope_ended);
    BOOST_TEST(!exception_thrown);
    BOOST_TEST(func_destroyed);
}

void check_cond()
{
    int n = 0;
    {
        int err = 0;
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
        err = -1;
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        int err = 0;
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        int err = 0;
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard{ normal_func(n), boost::scope::check_error_code(err), false };
        BOOST_TEST(!guard.active());
        err = -1;
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        std::error_code err{};
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< std::error_code > > guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
        err = std::make_error_code(std::errc::invalid_argument);
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        std::error_code err{};
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< std::error_code > > guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    try
    {
        int err = 0;
        boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > > guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
        throw std::runtime_error("error");
    }
    catch (...) {}
    BOOST_TEST_EQ(n, 0); // exception is not the failure condition, err was still 0 when the scope guard was destroyed
}

void check_deduction()
{
    int n = 0;
    {
        int err = 0;
        auto guard = boost::scope::make_scope_check(normal_func(n), boost::scope::check_error_code(err));
        BOOST_TEST(guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > >);
        err = -1;
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        int err = 0;
        auto guard = boost::scope::make_scope_check(normal_func(n), boost::scope::check_error_code(err), false);
        BOOST_TEST(!guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > >);
        err = -1;
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        int err = 0;
        const normal_func func{ n };
        const auto cond = boost::scope::check_error_code(err);
        auto guard = boost::scope::make_scope_check(func, cond, true);
        BOOST_TEST(guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > >);
        err = -1;
    }
    BOOST_TEST_EQ(n, 1);

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
    n = 0;
    {
        int err = 0;
        boost::scope::scope_check guard{ normal_func(n), boost::scope::check_error_code(err) };
        BOOST_TEST(guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > >);
        err = -1;
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        int err = 0;
        boost::scope::scope_check guard{ normal_func(n), boost::scope::error_code_checker(err), false };
        BOOST_TEST(!guard.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard), boost::scope::scope_check< normal_func, boost::scope::error_code_checker< int > >);
        err = -1;
    }
    BOOST_TEST_EQ(n, 0);

    n = 0;
    {
        int err = -1;
        boost::scope::scope_check guard([&n] { ++n; }, [&err]() noexcept { return err < 0; });
        BOOST_TEST(guard.active());
        err = -1;
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    {
        boost::scope::scope_check guard1{ normal_func(n), always_true() };
        boost::scope::scope_check guard2 = std::move(guard1);
        BOOST_TEST(guard2.active());
        BOOST_TEST_TRAIT_SAME(decltype(guard2), boost::scope::scope_check< normal_func, always_true >);
    }
    BOOST_TEST_EQ(n, 1);
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
}

int main()
{
    check_normal();
    check_throw();
    check_cond();
    check_deduction();

    return boost::report_errors();
}
