/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/unique_resource_fwd.hpp
 *
 * This header contains forward declaration of \c unique_resource template.
 */

#ifndef BOOST_SCOPE_UNIQUE_RESOURCE_FWD_HPP_INCLUDED_
#define BOOST_SCOPE_UNIQUE_RESOURCE_FWD_HPP_INCLUDED_

#include <type_traits>
#include <boost/config.hpp>
#include <boost/scope/detail/move_or_copy_construct_ref.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {

/*!
 * \brief RAII wrapper for automatically reclaiming arbitrary resources.
 */
template< typename Resource, typename Deleter, typename Traits = void >
class unique_resource;

/*!
 * \brief Checks if the resource is valid and creates a \c unique_resource wrapper.
 *
 * If the resource \a res is not equal to \a invalid, creates a unique resource wrapper
 * that is in allocated state. Otherwise creates a unique resource wrapper in deallocated state.
 *
 * \param res Resource to wrap.
 * \param invalid An invalid value for the resource.
 * \param del A deleter to invoke on the resource to reclaim it.
 *
 * \note This function does not call \a del if \a res is equal to \a invalid.
 */
template< typename Resource, typename Deleter, typename Invalid = typename std::decay< Resource >::type >
unique_resource< typename std::decay< Resource >::type, typename std::decay< Deleter >::type >
make_unique_resource_checked(Resource&& res, Invalid const& invalid, Deleter&& del)
    noexcept(detail::conjunction<
        std::is_nothrow_constructible< typename std::decay< Resource >::type, typename detail::move_or_copy_construct_ref< Resource, typename std::decay< Resource >::type >::type >,
        std::is_nothrow_constructible< typename std::decay< Deleter >::type, typename detail::move_or_copy_construct_ref< Deleter, typename std::decay< Deleter >::type >::type >
    >::value);

} // namespace scope

using scope::unique_resource;
using scope::make_unique_resource_checked;

} // namespace boost

#endif // BOOST_SCOPE_UNIQUE_RESOURCE_FWD_HPP_INCLUDED_
