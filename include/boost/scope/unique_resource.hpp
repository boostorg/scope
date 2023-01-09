/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2022 Andrey Semashev
 */
/*!
 * \file scope/unique_resource.hpp
 *
 * This header contains definition of \c unique_resource template.
 */

#ifndef BOOST_SCOPE_UNIQUE_RESOURCE_HPP_INCLUDED_
#define BOOST_SCOPE_UNIQUE_RESOURCE_HPP_INCLUDED_

#include <utility> // std::swap
#include <type_traits>
#include <boost/config.hpp>
#include <boost/core/addressof.hpp>
#include <boost/scope/unique_resource_fwd.hpp>
#include <boost/scope/detail/compact_storage.hpp>
#include <boost/scope/detail/move_or_copy_ref.hpp>
#include <boost/scope/detail/type_traits/void_t.hpp>
#include <boost/scope/detail/type_traits/is_swappable.hpp>
#include <boost/scope/detail/type_traits/is_nothrow_swappable.hpp>
#include <boost/scope/detail/type_traits/negation.hpp>
#include <boost/scope/detail/type_traits/conjunction.hpp>
#include <boost/scope/detail/type_traits/disjunction.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {
namespace scope {
namespace detail {

template< typename T >
class ref_wrapper
{
private:
    T* m_value;

public:
    explicit
#if !defined(BOOST_CORE_NO_CONSTEXPR_ADDRESSOF)
    constexpr
#endif
    ref_wrapper(T& value) noexcept :
        m_value(boost::addressof(value))
    {
    }

    ref_wrapper& operator= (T& value) noexcept
    {
        m_value = boost::addressof(value);
        return *this;
    }

    ref_wrapper(T&&) = delete;
    ref_wrapper& operator= (T&&) = delete;

    operator T& () const noexcept
    {
        return *m_value;
    }
};

template< typename Resource >
struct make_internal_resource_type
{
    typedef Resource type;
};

template< typename Resource >
struct make_internal_resource_type< Resource& >
{
    typedef ref_wrapper< Resource > type;
};

template< typename Resource, typename Traits, typename = void >
struct has_custom_default : public std::false_type { };
template< typename Resource, typename Traits >
struct has_custom_default< Resource, Traits, detail::void_t< decltype(Traits::make_default()) > > :
    public std::is_constructible< Resource, decltype(Traits::make_default()) >
{
};

template< typename Resource, typename Traits, typename = void >
struct has_deallocated_state : public std::false_type { };
template< typename Resource, typename Traits >
struct has_deallocated_state< Resource, Traits, detail::void_t< decltype(!!Traits::is_allocated(std::declval< Resource& >())) > > :
    public std::true_type
{
};

template< typename Resource, typename Traits, bool = has_custom_default< Resource, Traits >::value >
struct resource_holder :
    public detail::compact_storage< typename make_internal_resource_type< Resource >::type >
{
    typedef Resource resource_type;
    typedef Traits traits_type;
    typedef detail::compact_storage< typename make_internal_resource_type< Resource >::type > resource_base;

    template<
        bool Requires = std::is_default_constructible< resource_type >::value,
        typename = typename std::enable_if< Requires >::type
    >
    constexpr resource_holder() noexcept(std::is_nothrow_default_constructible< resource_type >::value) :
        resource_base()
    {
    }

    template<
        typename R,
        typename = typename std::enable_if< std::is_constructible< resource_type, R >::value >::type
    >
    explicit resource_holder(R&& res)
        noexcept(std::is_nothrow_constructible< resource_type, R >::value) :
        resource_base(static_cast< R&& >(res))
    {
    }

    template<
        typename R,
        typename D,
        typename = typename std::enable_if< std::is_constructible< resource_type, R >::value >::type
    >
    explicit resource_holder(R&& res, D&& del, bool allocated)
        noexcept(std::is_nothrow_constructible< resource_type, R >::value) try :
        resource_base(static_cast< R&& >(res))
    {
    }
    catch (...)
    {
        if (allocated)
            del(static_cast< R&& >(res));
    }

    resource_type& get() noexcept
    {
        return resource_base::get();
    }

    resource_type const& get() const noexcept
    {
        return resource_base::get();
    }
};

template< typename Resource, typename Traits >
struct resource_holder< Resource, Traits, true > :
    public detail::compact_storage< typename make_internal_resource_type< Resource >::type >
{
    typedef Resource resource_type;
    typedef Traits traits_type;
    typedef detail::compact_storage< typename make_internal_resource_type< Resource >::type > resource_base;

    constexpr resource_holder()
        noexcept(detail::conjunction<
            std::integral_constant< bool, noexcept(traits_type::make_default()) >,
            std::is_nothrow_constructible< resource_type, decltype(traits_type::make_default()) >
        >::value) :
        resource_base(traits_type::make_default())
    {
    }

    template<
        typename R,
        typename = typename std::enable_if< std::is_constructible< resource_type, R >::value >::type
    >
    explicit resource_holder(R&& res)
        noexcept(std::is_nothrow_constructible< resource_type, R >::value) :
        resource_base(static_cast< R&& >(res))
    {
    }

    template<
        typename R,
        typename D,
        typename = typename std::enable_if< std::is_constructible< resource_type, R >::value >::type
    >
    explicit resource_holder(R&& res, D&& del, bool allocated)
        noexcept(std::is_nothrow_constructible< resource_type, R >::value) try :
        resource_base(static_cast< R&& >(res))
    {
    }
    catch (...)
    {
        if (allocated)
            del(static_cast< R&& >(res));
    }

    resource_type& get() noexcept
    {
        return resource_base::get();
    }

    resource_type const& get() const noexcept
    {
        return resource_base::get();
    }
};

template< typename Resource, typename Deleter >
struct deleter_holder :
    public detail::compact_storage< Deleter >
{
public:
    typedef Resource resource_type;
    typedef Deleter deleter_type;
    typedef detail::compact_storage< deleter_type > deleter_base;

    template<
        bool Requires = std::is_default_constructible< deleter_type >::value,
        typename = typename std::enable_if< Requires >::type
    >
    constexpr deleter_holder() noexcept(std::is_nothrow_default_constructible< deleter_type >::value) :
        deleter_base()
    {
    }

    template<
        typename D,
        typename = typename std::enable_if< std::is_constructible< deleter_type, D >::value >::type
    >
    explicit deleter_holder(D&& del, resource_type& res, bool allocated)
        noexcept(std::is_nothrow_constructible< deleter_type, D >::value) try :
        deleter_base(static_cast< D&& >(del))
    {
    }
    catch (...)
    {
        if (BOOST_LIKELY(allocated))
            del(res);
    }
};

template< typename Resource, typename Traits, bool = detail::conjunction< has_deallocated_state< Resource, Traits >, has_custom_default< Resource, Traits > >::value >
struct use_deallocated_state : public std::false_type { };

template< typename Resource, typename Traits >
struct use_deallocated_state< Resource, Traits, true > :
    public detail::conjunction<
        std::integral_constant< bool, noexcept(Traits::make_default()) >,
        std::is_nothrow_assignable< Resource, decltype(Traits::make_default()) >
    >
{
};

template< typename Resource, typename Deleter, typename Traits, bool = use_deallocated_state< Resource, Traits >::value >
class unique_resource_data :
    public detail::resource_holder< Resource, Traits >,
    public detail::deleter_holder< Resource, Deleter >
{
public:
    typedef Resource resource_type;
    typedef Deleter deleter_type;
    typedef Traits traits_type;
    typedef detail::resource_holder< resource_type, traits_type > resource_holder;
    typedef detail::deleter_holder< resource_type, deleter_type > deleter_holder;

private:
    bool m_allocated;

public:
    template<
        bool Requires = detail::conjunction< std::is_default_constructible< resource_holder >, std::is_default_constructible< deleter_holder > >::value,
        typename = typename std::enable_if< Requires >::type
    >
    constexpr unique_resource_data()
        noexcept(detail::conjunction< std::is_nothrow_default_constructible< resource_holder >, std::is_nothrow_default_constructible< deleter_holder > >::value) :
        resource_holder(),
        deleter_holder(),
        m_allocated(false)
    {
    }

    unique_resource_data(unique_resource_data const&) = delete;
    unique_resource_data& operator= (unique_resource_data const&) = delete;

    template<
        bool Requires = detail::conjunction<
            std::is_constructible< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_constructible< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value,
        typename = typename std::enable_if< Requires >::type
    >
    unique_resource_data(unique_resource_data&& that)
        noexcept(detail::conjunction<
            std::is_nothrow_constructible< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_constructible< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value) try :
        resource_holder(static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource())),
        deleter_holder(static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter()), resource_holder::get(),
            !std::is_nothrow_move_constructible< resource_type >::value && that.m_allocated), // don't deallocate if the resource was copied
        m_allocated(that.m_allocated)
    {
        that.m_allocated = false;
    }
    catch (...)
    {
        BOOST_IF_CONSTEXPR (std::is_nothrow_move_constructible< resource_type >::value)
        {
            // The resource was moved to this object, and the deleter constructor failed with an exception.
            // The deleter holder has invoked the deleter already, so the move source is no longer valid.
            that.m_allocated = false;
        }
    }

    template<
        typename R,
        typename D,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< resource_holder, R, D, bool >,
            std::is_constructible< deleter_holder, D, resource_type&, bool >
        >::value >::type
    >
    explicit unique_resource_data(R&& res, D&& del, bool allocated)
        noexcept(detail::conjunction<
            std::is_nothrow_constructible< resource_holder, R, D, bool >,
            std::is_nothrow_constructible< deleter_holder, D, resource_type&, bool >
        >::value) :
        resource_holder(static_cast< R&& >(res), static_cast< D&& >(del), allocated),
        deleter_holder(static_cast< D&& >(del), resource_holder::get(), allocated),
        m_allocated(allocated)
    {
    }

    template<
        bool Requires = detail::conjunction<
            std::is_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value
    >
    typename std::enable_if< Requires, unique_resource_data& >::type operator= (unique_resource_data&& that)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        assign(static_cast< unique_resource_data&& >(that), typename std::is_nothrow_move_assignable< deleter_type >::type());
        return *this;
    }

    resource_type& get_resource() noexcept
    {
        return resource_holder::get();
    }

    resource_type const& get_resource() const noexcept
    {
        return resource_holder::get();
    }

    deleter_type& get_deleter() noexcept
    {
        return deleter_holder::get();
    }

    deleter_type const& get_deleter() const noexcept
    {
        return deleter_holder::get();
    }

    bool is_allocated() const noexcept
    {
        return m_allocated;
    }

    void set_deallocated() noexcept
    {
        m_allocated = false;
    }

    template< typename R >
    void assign_resource(R&& res) noexcept(std::is_nothrow_assignable< resource_type, R >::value)
    {
        get_resource() = static_cast< R&& >(res);
        m_allocated = true;
    }

    template< bool Requires = detail::conjunction< detail::is_swappable< resource_type >, detail::is_swappable< deleter_type > >::value >
    typename std::enable_if< Requires >::type swap(unique_resource_data& that)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        swap_impl(that, std::integral_constant< bool, detail::is_nothrow_swappable< resource_type >::value >());
    }

private:
    void assign(unique_resource_data&& that, std::true_type)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        get_resource() = static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource());
        get_deleter() = static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter());

        m_allocated = that.m_allocated;
        that.m_allocated = false;
    }

    void assign(unique_resource_data&& that, std::false_type)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        get_deleter() = static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter());
        get_resource() = static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource());

        m_allocated = that.m_allocated;
        that.m_allocated = false;
    }

    void swap_impl(unique_resource_data& that, std::true_type)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        using namespace std;

        swap(get_resource(), that.get_resource());
        try
        {
            swap(get_deleter(), that.get_deleter());
        }
        catch (...)
        {
            swap(get_resource(), that.get_resource());
            throw;
        }

        swap(m_allocated, that.m_allocated);
    }

    void swap_impl(unique_resource_data& that, std::false_type)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        using namespace std;

        swap(get_deleter(), that.get_deleter());
        try
        {
            swap(get_resource(), that.get_resource());
        }
        catch (...)
        {
            swap(get_deleter(), that.get_deleter());
            throw;
        }

        swap(m_allocated, that.m_allocated);
    }
};

template< typename Resource, typename Deleter, typename Traits >
class unique_resource_data< Resource, Deleter, Traits, true > :
    public detail::resource_holder< Resource, Traits >,
    public detail::deleter_holder< Resource, Deleter >
{
public:
    typedef Resource resource_type;
    typedef Deleter deleter_type;
    typedef Traits traits_type;
    typedef detail::resource_holder< resource_type, traits_type > resource_holder;
    typedef detail::deleter_holder< resource_type, deleter_type > deleter_holder;

public:
    template<
        bool Requires = detail::conjunction< std::is_default_constructible< resource_holder >, std::is_default_constructible< deleter_holder > >::value,
        typename = typename std::enable_if< Requires >::type
    >
    constexpr unique_resource_data()
        noexcept(detail::conjunction< std::is_nothrow_default_constructible< resource_holder >, std::is_nothrow_default_constructible< deleter_holder > >::value) :
        resource_holder(),
        deleter_holder()
    {
    }

    unique_resource_data(unique_resource_data const&) = delete;
    unique_resource_data& operator= (unique_resource_data const&) = delete;

    unique_resource_data(unique_resource_data&& that)
        noexcept(detail::conjunction<
            std::is_nothrow_constructible< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_constructible< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value) try :
        resource_holder(static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource())),
        deleter_holder(static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter()), resource_holder::get(),
            !std::is_nothrow_move_constructible< resource_type >::value && that.is_allocated()) // don't deallocate if the resource was copied
    {
        that.set_deallocated();
    }
    catch (...)
    {
        BOOST_IF_CONSTEXPR (std::is_nothrow_move_constructible< resource_type >::value)
        {
            // The resource was moved to this object, and the deleter constructor failed with an exception.
            // The deleter holder has invoked the deleter already, so the move source is no longer valid.
            that.set_deallocated();
        }
    }

    template<
        typename R,
        typename D,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< resource_holder, R, D, bool >,
            std::is_constructible< deleter_holder, D, resource_type&, bool >
        >::value >::type
    >
    explicit unique_resource_data(R&& res, D&& del, bool allocated)
        noexcept(detail::conjunction<
            std::is_nothrow_constructible< resource_holder, R, D, bool >,
            std::is_nothrow_constructible< deleter_holder, D, resource_type&, bool >
        >::value) :
        unique_resource_data(static_cast< R&& >(res), static_cast< D&& >(del), allocated,
            !has_deallocated_state< resource_type, traits_type >::value || traits_type::is_allocated(static_cast< R&& >(res)))
    {
    }

    template<
        bool Requires = detail::conjunction<
            std::is_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value
    >
    typename std::enable_if< Requires, unique_resource_data& >::type operator= (unique_resource_data&& that)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        assign(static_cast< unique_resource_data&& >(that), typename std::is_nothrow_move_assignable< deleter_type >::type());
        return *this;
    }

    resource_type& get_resource() noexcept
    {
        return resource_holder::get();
    }

    resource_type const& get_resource() const noexcept
    {
        return resource_holder::get();
    }

    deleter_type& get_deleter() noexcept
    {
        return deleter_holder::get();
    }

    deleter_type const& get_deleter() const noexcept
    {
        return deleter_holder::get();
    }

    bool is_allocated() const noexcept
    {
        return traits_type::is_allocated(get_resource());
    }

    void set_deallocated() noexcept
    {
        get_resource() = traits_type::make_default();
    }

    template< typename R >
    void assign_resource(R&& res) noexcept(std::is_nothrow_assignable< resource_type, R >::value)
    {
        get_resource() = static_cast< R&& >(res);
    }

    template< bool Requires = detail::conjunction< detail::is_swappable< resource_type >, detail::is_swappable< deleter_type > >::value >
    typename std::enable_if< Requires >::type swap(unique_resource_data& that)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        swap_impl(that, std::integral_constant< bool, detail::is_nothrow_swappable< resource_type >::value >());
    }

private:
    template<
        typename R,
        typename D,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< resource_holder, R, D, bool >,
            std::is_constructible< deleter_holder, D, resource_type&, bool >
        >::value >::type
    >
    explicit unique_resource_data(R&& res, D&& del, bool, bool res_allocated)
        noexcept(detail::conjunction<
            std::is_nothrow_constructible< resource_holder, R, D, bool >,
            std::is_nothrow_constructible< deleter_holder, D, resource_type&, bool >
        >::value) :
        resource_holder(static_cast< R&& >(res), static_cast< D&& >(del), res_allocated),
        deleter_holder(static_cast< D&& >(del), resource_holder::get(), res_allocated)
    {
    }

    void assign(unique_resource_data&& that, std::true_type)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        get_resource() = static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource());
        get_deleter() = static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter());

        that.set_deallocated();
    }

    void assign(unique_resource_data&& that, std::false_type)
        noexcept(detail::conjunction<
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< resource_type, resource_type >::type >,
            std::is_nothrow_assignable< deleter_type, typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >
        >::value)
    {
        get_deleter() = static_cast< typename detail::move_or_copy_ref< deleter_type, deleter_type >::type >(that.get_deleter());
        get_resource() = static_cast< typename detail::move_or_copy_ref< resource_type, resource_type >::type >(that.get_resource());

        that.set_deallocated();
    }

    void swap_impl(unique_resource_data& that, std::true_type)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        using namespace std;

        swap(get_resource(), that.get_resource());
        try
        {
            swap(get_deleter(), that.get_deleter());
        }
        catch (...)
        {
            swap(get_resource(), that.get_resource());
            throw;
        }
    }

    void swap_impl(unique_resource_data& that, std::false_type)
        noexcept(detail::conjunction< detail::is_nothrow_swappable< resource_type >, detail::is_nothrow_swappable< deleter_type > >::value)
    {
        using namespace std;

        swap(get_deleter(), that.get_deleter());
        try
        {
            swap(get_resource(), that.get_resource());
        }
        catch (...)
        {
            swap(get_deleter(), that.get_deleter());
            throw;
        }
    }
};

template< typename T, typename = void >
struct is_dereferenceable : public std::false_type { };
template< >
struct is_dereferenceable< void*, void > : public std::false_type { };
template< >
struct is_dereferenceable< const void*, void > : public std::false_type { };
template< >
struct is_dereferenceable< volatile void*, void > : public std::false_type { };
template< >
struct is_dereferenceable< const volatile void*, void > : public std::false_type { };
template< >
struct is_dereferenceable< void*&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const void*&, void > : public std::false_type { };
template< >
struct is_dereferenceable< volatile void*&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const volatile void*&, void > : public std::false_type { };
template< >
struct is_dereferenceable< void* const&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const void* const&, void > : public std::false_type { };
template< >
struct is_dereferenceable< volatile void* const&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const volatile void* const&, void > : public std::false_type { };
template< >
struct is_dereferenceable< void* volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const void* volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< volatile void* volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const volatile void* volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< void* const volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const void* const volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< volatile void* const volatile&, void > : public std::false_type { };
template< >
struct is_dereferenceable< const volatile void* const volatile&, void > : public std::false_type { };
template< typename T >
struct is_dereferenceable< T, detail::void_t< decltype(*std::declval< T const& >()) > > : public std::true_type { };

} // namespace detail

/*!
 * \brief RAII wrapper for automatically reclaiming arbitrary resources.
 */
template< typename Resource, typename Deleter, typename Traits >
class unique_resource
{
public:
    //! Resource type
    typedef Resource resource_type;
    //! Deleter type
    typedef Deleter deleter_type;
    //! Resource traits
    typedef Traits traits_type;

private:
    typedef detail::unique_resource_data< resource_type, deleter_type, traits_type > data;
    data m_data;

public:
    //! Constructs an inactive unique resource guard.
    template<
        bool Requires = std::is_default_constructible< data >::value,
        typename = typename std::enable_if< Requires >::type
    >
    constexpr unique_resource() noexcept(std::is_nothrow_default_constructible< data >::value)
    {
    }

    //! Constructs a unique resource guard with the given resource and deleter.
    template<
        typename R,
        typename D,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< data, typename detail::move_or_copy_ref< R, resource_type >::type, typename detail::move_or_copy_ref< D, deleter_type >::type, bool >,
            detail::negation< detail::disjunction< std::is_reference< resource_type >, std::is_reference< R > > > // prevent binding lvalue-reference resource to an rvalue
        >::value >::type
    >
    unique_resource(R&& res, D&& del)
        noexcept(std::is_nothrow_constructible<
            data,
            typename detail::move_or_copy_ref< R, resource_type >::type,
            typename detail::move_or_copy_ref< D, deleter_type >::type,
            bool
        >::value) :
        m_data(static_cast< typename detail::move_or_copy_ref< R, resource_type >::type >(res), static_cast< typename detail::move_or_copy_ref< D, deleter_type >::type >(del), true)
    {
    }

    unique_resource(unique_resource const&) = delete;
    unique_resource& operator= (unique_resource const&) = delete;

    template<
        bool Requires = std::is_move_constructible< data >::value,
        typename = typename std::enable_if< Requires >::type
    >
    unique_resource(unique_resource&& that) noexcept(std::is_nothrow_move_constructible< data >::value) :
        m_data(static_cast< data&& >(that.m_data))
    {
    }

    template< bool Requires = std::is_move_assignable< data >::value >
    typename std::enable_if< Requires, unique_resource& >::type operator= (unique_resource&& that)
        noexcept(std::is_nothrow_move_assignable< data >::value)
    {
        reset();
        m_data = static_cast< data&& >(that.m_data);
        return *this;
    }

    //! Invokes <tt>reset()</tt> and destroys the resource.
    ~unique_resource() noexcept(noexcept(std::declval< deleter_type& >()(std::declval< resource_type& >())))
    {
        reset();
    }

    //! Returns \c true if the resource is allocated and to be reclaimed by calling the deleter, otherwise \c false.
    bool allocated() const noexcept
    {
        return m_data.is_allocated();
    }

    //! Returns a reference to the resource.
    resource_type const& get() const noexcept
    {
        return m_data.get_resource();
    }

    //! Returns a reference to the deleter.
    deleter_type const& get_deleter() const noexcept
    {
        return m_data.get_deleter();
    }

    //! Marks the resource as deallocated. Does not call the deleter if the resource was previously allocated.
    void release() noexcept
    {
        m_data.set_deallocated();
    }

    //! If the resource is allocated, calls the deleter function on it and marks the resource as deallocated.
    void reset() noexcept(noexcept(std::declval< deleter_type& >()(std::declval< resource_type& >())))
    {
        if (BOOST_LIKELY(m_data.is_allocated()))
        {
            m_data.get_deleter()(m_data.get_resource());
            m_data.set_deallocated();
        }
    }

    //! If the resource is allocated, calls the deleter function on it and marks the resource as deallocated.
    template< typename R >
    typename std::enable_if< detail::conjunction<
        std::is_assignable< resource_type, typename detail::move_or_copy_ref< R, resource_type >::type >,
        detail::negation< detail::disjunction< std::is_reference< resource_type >, std::is_reference< R > > > // prevent binding lvalue-reference resource to an rvalue
    >::value >::type reset(R&& res)
        noexcept(detail::conjunction<
            std::integral_constant< bool, noexcept(std::declval< deleter_type& >()(std::declval< resource_type& >())) >,
            std::is_nothrow_assignable< resource_type, typename detail::move_or_copy_ref< R, resource_type >::type >
        >::value)
    {
        reset();

        try
        {
            m_data.assign_resource(static_cast< typename detail::move_or_copy_ref< R, resource_type >::type >(res));
        }
        catch (...)
        {
            m_data.get_deleter()(static_cast< typename detail::move_or_copy_ref< R, resource_type >::type >(res));
            throw;
        }
    }

    template< bool Requires = detail::is_dereferenceable< resource_type >::value >
    typename std::enable_if< Requires, resource_type const& >::type operator-> () const noexcept
    {
        return get();
    }

    template< bool Requires = detail::is_dereferenceable< resource_type >::value >
    typename std::enable_if< Requires, decltype(*std::declval< resource_type const& >()) >::type
    operator* () const noexcept(noexcept(*std::declval< resource_type const& >()))
    {
        return *get();
    }

    //! Swaps two unique resource wrappers
    template< bool Requires = detail::is_swappable< data >::value >
    typename std::enable_if< Requires >::type swap(unique_resource& that) noexcept(detail::is_nothrow_swappable< data >::value)
    {
        m_data.swap(that.m_data);
    }

    //! Swaps two unique resource wrappers
    template< bool Requires = detail::is_swappable< data >::value >
    friend typename std::enable_if< Requires >::type
    swap(unique_resource& left, unique_resource& right) noexcept(detail::is_nothrow_swappable< data >::value)
    {
        left.swap(right);
    }

private:
    //! Constructs a unique resource guard with the given resource and deleter.
    template<
        typename R,
        typename D,
        typename = typename std::enable_if< detail::conjunction<
            std::is_constructible< data, typename detail::move_or_copy_ref< R, resource_type >::type, typename detail::move_or_copy_ref< D, deleter_type >::type, bool >,
            detail::negation< detail::disjunction< std::is_reference< resource_type >, std::is_reference< R > > > // prevent binding lvalue-reference resource to an rvalue
        >::value >::type
    >
    unique_resource(R&& res, D&& del, bool allocated)
        noexcept(std::is_nothrow_constructible<
            data,
            typename detail::move_or_copy_ref< R, resource_type >::type,
            typename detail::move_or_copy_ref< D, deleter_type >::type,
            bool
        >::value) :
        m_data(static_cast< typename detail::move_or_copy_ref< R, resource_type >::type >(res), static_cast< typename detail::move_or_copy_ref< D, deleter_type >::type >(del), allocated)
    {
    }

    template< typename Res, typename Del, typename Inv >
    friend unique_resource< typename std::decay< Res >::type, typename std::decay< Del >::type >
    make_unique_resource_checked(Res&& res, Inv const& invalid, Del&& del)
        noexcept(std::is_nothrow_constructible< unique_resource< typename std::decay< Res >::type, typename std::decay< Del >::type >, Res, Del >::value);
};

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
template< typename Resource, typename Deleter >
unique_resource(Resource, Deleter) -> unique_resource< Resource, Deleter >;
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)

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
inline unique_resource< typename std::decay< Resource >::type, typename std::decay< Deleter >::type >
make_unique_resource_checked(Resource&& res, Invalid const& invalid, Deleter&& del)
    noexcept(std::is_nothrow_constructible< unique_resource< typename std::decay< Resource >::type, typename std::decay< Deleter >::type >, Resource, Deleter >::value)
{
    return unique_resource< typename std::decay< Resource >::type, typename std::decay< Deleter >::type >(
        static_cast< Resource&& >(res), static_cast< Deleter&& >(del), res == invalid ? false : true);
}

} // namespace scope
} // namespace boost

#endif // BOOST_SCOPE_UNIQUE_RESOURCE_HPP_INCLUDED_
