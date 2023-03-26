/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/exception_checker.hpp
 *
 * This header contains definition of \c exception_checker type.
 */

#ifndef BOOST_SCOPE_EXCEPTION_CHECKER_HPP_INCLUDED_
#define BOOST_SCOPE_EXCEPTION_CHECKER_HPP_INCLUDED_

#include <boost/scope/detail/config.hpp>
#include <boost/core/uncaught_exceptions.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

/*!
 * \brief A predicate for checking whether an exception is being thrown.
 *
 * On construction, the predicate captures the current number of uncaught exceptions,
 * which it then uses when called to detect if a new exception is being thrown.
 */
class exception_checker
{
public:
    //! Predicate result type
    typedef bool result_type;

private:
    unsigned int m_uncaught_count;

public:
    /*!
     * \brief Constructs the predicate.
     *
     * Upon construction, the predicate saves the current number of uncaught exceptions.
     * This information will be used when calling the predicate to detect if a new
     * exception is being thrown.
     *
     * **Throws:** Nothing.
     */
    exception_checker() noexcept :
        m_uncaught_count(boost::core::uncaught_exceptions())
    {
    }

    /*!
     * \brief Checks if an exception is being thrown.
     *
     * **Throws:** Nothing.
     *
     * \returns \c true if the number of uncaught exceptions at the point of call is
     *          greater than that at the point of construction of the predicate,
     *          otherwise \c false.
     */
    result_type operator()() const noexcept
    {
        return boost::core::uncaught_exceptions() > m_uncaught_count;
    }
};

/*!
 * \brief Creates a predicate for checking whether an exception is being thrown
 *
 * **Throws:** Nothing.
 */
inline exception_checker check_exception() noexcept
{
    return exception_checker();
}

} // namespace scope
} // namespace boost

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_EXCEPTION_CHECKER_HPP_INCLUDED_