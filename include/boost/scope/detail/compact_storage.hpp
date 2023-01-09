/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/detail/compact_storage.hpp
 *
 * This header contains utility helpers for implementing compact storage
 * for class members. In particular, it allows to leverage empty base optimization (EBO).
 */

#ifndef BOOST_SCOPE_DETAIL_COMPACT_STORAGE_HPP_INCLUDED_
#define BOOST_SCOPE_DETAIL_COMPACT_STORAGE_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>
#include <boost/scope/detail/type_traits/is_final.hpp>
#include <boost/scope/detail/type_traits/negation.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {
namespace detail {

template< typename T >
class compact_storage_base :
    private T
{
public:
    template< typename... Args >
    constexpr compact_storage_base(Args&&... args) noexcept(std::is_nothrow_constructible< T, Args... >::value) :
        T(static_cast< Args&& >(args)...)
    {
    }

    compact_storage_base(compact_storage_base&&) = default;
    compact_storage_base& operator= (compact_storage_base&&) = default;

    compact_storage_base(compact_storage_base const&) = default;
    compact_storage_base& operator= (compact_storage_base const&) = default;

    T& get() noexcept
    {
        return *static_cast< T* >(this);
    }

    T const& get() const noexcept
    {
        return *static_cast< const T* >(this);
    }
};

template< typename T >
class compact_storage_member
{
private:
    T m_data;

public:
    template< typename... Args >
    constexpr compact_storage_member(Args&&... args) noexcept(std::is_nothrow_constructible< T, Args... >::value) :
        m_data(static_cast< Args&& >(args)...)
    {
    }

    compact_storage_member(compact_storage_member&&) = default;
    compact_storage_member& operator= (compact_storage_member&&) = default;

    compact_storage_member(compact_storage_member const&) = default;
    compact_storage_member& operator= (compact_storage_member const&) = default;

    T& get() noexcept
    {
        return m_data;
    }

    T const& get() const noexcept
    {
        return m_data;
    }
};

//! The class allows to place data members in the tail padding of type \a T if the user's class derives from it
template< typename T, typename Tag = void >
class compact_storage :
    public std::conditional<
        detail::conjunction< std::is_class< T >, detail::negation< detail::is_final< T > > >::value,
        compact_storage_base< T >,
        compact_storage_member< T >
    >::type
{
private:
    typedef typename std::conditional<
        detail::conjunction< std::is_class< T >, detail::negation< detail::is_final< T > > >::value,
        compact_storage_base< T >,
        compact_storage_member< T >
    >::type base_type;

public:
    template< typename... Args >
    constexpr compact_storage(Args&&... args) noexcept(std::is_nothrow_constructible< T, Args... >::value) :
        base_type(static_cast< Args&& >(args)...)
    {
    }

    compact_storage(compact_storage&&) = default;
    compact_storage& operator= (compact_storage&&) = default;

    compact_storage(compact_storage const&) = default;
    compact_storage& operator= (compact_storage const&) = default;
};

} // namespace detail
} // namespace scope
} // namespace boost

#endif // BOOST_SCOPE_DETAIL_COMPACT_STORAGE_HPP_INCLUDED_
