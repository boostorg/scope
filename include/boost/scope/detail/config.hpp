/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * https://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2023 Andrey Semashev
 */
/*!
 * \file scope/detail/config.hpp
 *
 * This header contains Boost.Scope common configuration.
 */

#ifndef BOOST_SCOPE_DETAIL_CONFIG_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_CONFIG_HPP_INCLUDED_

#include <boost/config.hpp>
#include <boost/scope/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined(BOOST_SCOPE_DETAIL_DOC_HIDDEN)
#if !defined(BOOST_SCOPE_DOXYGEN)
#define BOOST_SCOPE_DETAIL_DOC_HIDDEN(...) __VA_ARGS__
#else
#define BOOST_SCOPE_DETAIL_DOC_HIDDEN(...) ...
#endif
#endif

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_DETAIL_CONFIG_HPP_INCLUDED_
