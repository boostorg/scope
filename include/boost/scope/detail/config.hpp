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

#if !(defined(__cpp_noexcept_function_type) && __cpp_noexcept_function_type >= 201510l) && !defined(_NOEXCEPT_TYPES_SUPPORTED)
#define BOOST_SCOPE_NO_CXX17_NOEXCEPT_FUNCTION_TYPES
#endif

#if !(defined(__cpp_nontype_template_parameter_auto) && __cpp_nontype_template_parameter_auto >= 201606l) && \
    !(defined(BOOST_GCC_VERSION) && BOOST_GCC_VERSION >= 70100 && __cplusplus >= 201703l) && \
    !(defined(BOOST_CLANG) && BOOST_CLANG_VERSION >= 40000 && __cplusplus >= 201406l /* non-standard value that is greater than 201402, which is reported by clang up to 4.0.0 for C++1z */) && \
    !(defined(BOOST_MSVC) && BOOST_MSVC >= 1914 && BOOST_CXX_VERSION >= 201703l)
#define BOOST_SCOPE_NO_CXX17_NONTYPE_TEMPLATE_PARAMETER_AUTO
#endif

#if defined(BOOST_NO_CXX17_FOLD_EXPRESSIONS) && \
    !(defined(BOOST_GCC_VERSION) && BOOST_GCC_VERSION >= 60100 && __cplusplus >= 201703l) && \
    !(defined(BOOST_CLANG) && BOOST_CLANG_VERSION >= 30900 && __cplusplus >= 201406l /* non-standard value that is greater than 201402, which is reported by clang up to 4.0.0 for C++1z */) && \
    !(defined(BOOST_MSVC) && BOOST_MSVC >= 1912 && BOOST_CXX_VERSION >= 201703l)
#define BOOST_SCOPE_NO_CXX17_FOLD_EXPRESSIONS
#endif

#if !defined(BOOST_SCOPE_DETAIL_DOC_ALT)
#if !defined(BOOST_SCOPE_DOXYGEN)
#define BOOST_SCOPE_DETAIL_DOC_ALT(alt, ...) __VA_ARGS__
#else
#define BOOST_SCOPE_DETAIL_DOC_ALT(alt, ...) alt
#endif
#endif

#if !defined(BOOST_SCOPE_DETAIL_DOC_HIDDEN)
#define BOOST_SCOPE_DETAIL_DOC_HIDDEN(...) BOOST_SCOPE_DETAIL_DOC_ALT(..., __VA_ARGS__)
#endif

#if !defined(BOOST_SCOPE_DETAIL_DOC)
#if !defined(BOOST_SCOPE_DOXYGEN)
#define BOOST_SCOPE_DETAIL_DOC(...)
#else
#define BOOST_SCOPE_DETAIL_DOC(...) __VA_ARGS__
#endif
#endif

#include <boost/scope/detail/footer.hpp>

#endif // BOOST_SCOPE_DETAIL_CONFIG_HPP_INCLUDED_
