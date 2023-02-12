/*
 *             Copyright Andrey Semashev 2023.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   unique_resource.cpp
 * \author Andrey Semashev
 *
 * \brief  This file contains tests for \c unique_resource.
 */

#include <boost/scope/unique_resource.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/lightweight_test_trait.hpp>
#include <boost/config.hpp>
#include <ostream>
#include <utility>
#include <stdexcept>

template< typename Resource >
struct empty_resource_deleter
{
    void operator() (Resource const& res) const noexcept
    {
    }
};

template< typename Resource >
class checking_resource_deleter
{
private:
    Resource* m_deleted;
    int& m_n;

public:
    explicit checking_resource_deleter(int& n) noexcept :
        m_deleted(nullptr),
        m_n(n)
    {
    }

    explicit checking_resource_deleter(Resource& deleted, int& n) noexcept :
        m_deleted(&deleted),
        m_n(n)
    {
    }

    checking_resource_deleter(checking_resource_deleter&& that) noexcept :
        m_deleted(that.m_deleted),
        m_n(that.m_n)
    {
    }

    checking_resource_deleter(checking_resource_deleter const& that) noexcept :
        m_deleted(that.m_deleted),
        m_n(that.m_n)
    {
    }

    // Make sure the deleter is move and copy-assignable
    checking_resource_deleter& operator= (checking_resource_deleter&& that) noexcept
    {
        m_deleted = that.m_deleted;
        return *this;
    }

    checking_resource_deleter& operator= (checking_resource_deleter const& that) noexcept
    {
        m_deleted = that.m_deleted;
        return *this;
    }

    Resource* get_deleted() const noexcept
    {
        return m_deleted;
    }

    void operator() (Resource const& res) const noexcept
    {
        if (m_deleted)
            *m_deleted = res;
        ++m_n;
    }
};

int g_n = 0, g_res1 = 0, g_res2 = 0;

void check_int()
{
    {
        boost::scope::unique_resource< int, empty_resource_deleter< int > > ur;
        BOOST_TEST_EQ(ur.get(), 0);
        BOOST_TEST(!ur.allocated());
    }

    {
        boost::scope::unique_resource< int, empty_resource_deleter< int > > ur{ 10 };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
    }

    int n = 0, deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur{ 0, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 0);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, 0);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        BOOST_TEST_EQ(ur.get_deleter().get_deleted(), &deleted_res1);
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, 10);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.release();
        BOOST_TEST(!ur.allocated());
    }
    BOOST_TEST_EQ(n, 0);
    BOOST_TEST_EQ(deleted_res1, -1);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.reset();
        BOOST_TEST(!ur.allocated());
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, 10);
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.reset(20);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, 10);
        deleted_res1 = -1;
        BOOST_TEST_EQ(ur.get(), 20);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 20);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur2{ std::move(ur1) };
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, 10);

    n = 0;
    deleted_res1 = -1;
    int deleted_res2 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur2{ 20, checking_resource_deleter< int >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST(ur2.allocated());
        ur2 = std::move(ur1);
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, -1);
        BOOST_TEST_EQ(deleted_res2, 20);
        deleted_res2 = -1;
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 10);
    BOOST_TEST_EQ(deleted_res2, -1);

    {
        boost::scope::unique_resource< int, empty_resource_deleter< int > > ur1;
        BOOST_TEST_EQ(ur1.get(), 0);
        BOOST_TEST(!ur1.allocated());
        boost::scope::unique_resource< int, empty_resource_deleter< int > > ur2{ 10, empty_resource_deleter< int >() };
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), 0);
        BOOST_TEST(!ur2.allocated());
    }

    n = 0;
    deleted_res1 = -1;
    deleted_res2 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int > > ur2{ 20, checking_resource_deleter< int >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(n, 0);
        BOOST_TEST_EQ(ur1.get(), 20);
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur2.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 10);
    BOOST_TEST_EQ(deleted_res2, 20);

    struct local
    {
        static void raw_func_deleter(int)
        {
            ++g_n;
        }

        static void raw_func_deleter1(int res)
        {
            g_res1 = res;
        }

        static void raw_func_deleter2(int res)
        {
            g_res2 = res;
        }
    };

    g_n = 0;
    {
        boost::scope::unique_resource< int, void (&)(int) > ur(10, local::raw_func_deleter);
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(g_n, 1);

    g_n = 0;
    {
        boost::scope::unique_resource< int, void (&)(int) > ur1(10, local::raw_func_deleter);
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, void (&)(int) > ur2(std::move(ur1));
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(g_n, 1);

    g_res1 = 0;
    g_res2 = 0;
    {
        boost::scope::unique_resource< int, void (&)(int) > ur1(10, local::raw_func_deleter1);
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, void (&)(int) > ur2(20, local::raw_func_deleter2);
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST(ur2.allocated());
        ur2 = std::move(ur1);
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(g_res1, 10);
    BOOST_TEST_EQ(g_res2, 20);

    g_res1 = 0;
    g_res2 = 0;
    {
        boost::scope::unique_resource< int, void (&)(int) > ur1(10, local::raw_func_deleter1);
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, void (&)(int) > ur2(20, local::raw_func_deleter2);
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(ur1.get(), 20);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
    }
    BOOST_TEST_EQ(g_res1, 10);
    BOOST_TEST_EQ(g_res2, 20);
}


struct struct_resource
{
    int value;

    struct_resource(int v = 0) noexcept :
        value(v)
    {
    }

    friend bool operator== (struct_resource const& left, struct_resource const& right) noexcept
    {
        return left.value == right.value;
    }

    friend bool operator!= (struct_resource const& left, struct_resource const& right) noexcept
    {
        return !(left == right);
    }

    friend std::ostream& operator<< (std::ostream& strm, struct_resource const& res)
    {
        strm << "{ " << res.value << " }";
        return strm;
    }
};

void check_struct()
{
    {
        boost::scope::unique_resource< struct_resource, empty_resource_deleter< struct_resource > > ur;
        BOOST_TEST_EQ(ur.get(), struct_resource{});
        BOOST_TEST(!ur.allocated());
    }

    int n = 0;
    struct_resource deleted_res1{ -1 };
    {
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur{ struct_resource{ 10 }, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), struct_resource{ 10 });
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    {
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur{ struct_resource{ 10 }, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), struct_resource{ 10 });
        BOOST_TEST(ur.allocated());
        ur.reset(20);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });
        deleted_res1 = struct_resource{ -1 };
        BOOST_TEST_EQ(ur.get(), struct_resource{ 20 });
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 20 });

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    {
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur1{ struct_resource{ 10 }, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), struct_resource{ 10 });
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur2{ std::move(ur1) };
        BOOST_TEST_EQ(ur2.get(), struct_resource{ 10 });
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    struct_resource deleted_res2{ -1 };
    {
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur1{ struct_resource{ 10 }, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), struct_resource{ 10 });
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur2{ struct_resource{ 20 }, checking_resource_deleter< struct_resource >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), struct_resource{ 20 });
        BOOST_TEST(ur2.allocated());
        ur2 = std::move(ur1);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, struct_resource{ -1 });
        BOOST_TEST_EQ(deleted_res2, struct_resource{ 20 });
        deleted_res2 = struct_resource{ -1 };
        BOOST_TEST_EQ(ur2.get(), struct_resource{ 10 });
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });
    BOOST_TEST_EQ(deleted_res2, struct_resource{ -1 });

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    deleted_res2 = struct_resource{ -1 };
    {
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur1{ struct_resource{ 10 }, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), struct_resource{ 10 });
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< struct_resource, checking_resource_deleter< struct_resource > > ur2{ struct_resource{ 20 }, checking_resource_deleter< struct_resource >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), struct_resource{ 20 });
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(n, 0);
        BOOST_TEST_EQ(ur1.get(), struct_resource{ 20 });
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), struct_resource{ 10 });
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur2.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });
    BOOST_TEST_EQ(deleted_res2, struct_resource{ 20 });
}


void check_ptr()
{
    {
        boost::scope::unique_resource< struct_resource*, empty_resource_deleter< struct_resource* > > ur;
        BOOST_TEST_EQ(ur.get(), static_cast< struct_resource* >(nullptr));
        BOOST_TEST(!ur.allocated());
    }

    int n = 0;
    struct_resource res1{ 10 };
    struct_resource* deleted_res1 = nullptr;
    {
        boost::scope::unique_resource< struct_resource*, checking_resource_deleter< struct_resource* > > ur{ &res1, checking_resource_deleter< struct_resource* >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), &res1);
        BOOST_TEST_EQ(ur.get()->value, 10);
        BOOST_TEST_EQ(ur->value, 10);
        BOOST_TEST_EQ((*ur).value, 10);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, &res1);
}


void check_ref()
{
    int n = 0;
    struct_resource res1{ 10 };
    struct_resource deleted_res1{ -1 };
    {
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur{ res1, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), res1);
        BOOST_TEST_EQ(&ur.get(), &res1);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, res1);

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    struct_resource res2{ 20 };
    {
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur{ res1, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(&ur.get(), &res1);
        BOOST_TEST(ur.allocated());
        ur.reset(res2);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, res1);
        deleted_res1 = struct_resource{ -1 };
        BOOST_TEST_EQ(&ur.get(), &res2);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, res2);

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    {
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur1{ res1, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(&ur1.get(), &res1);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur2{ std::move(ur1) };
        BOOST_TEST_EQ(&ur2.get(), &res1);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, res1);

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    struct_resource deleted_res2{ -1 };
    {
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur1{ res1, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(&ur1.get(), &res1);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur2{ res2, checking_resource_deleter< struct_resource >(deleted_res2, n) };
        BOOST_TEST_EQ(&ur2.get(), &res2);
        BOOST_TEST(ur2.allocated());
        ur2 = std::move(ur1);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, struct_resource{ -1 });
        BOOST_TEST_EQ(deleted_res2, struct_resource{ 20 });
        deleted_res2 = struct_resource{ -1 };
        BOOST_TEST_EQ(&ur2.get(), &res1);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });
    BOOST_TEST_EQ(deleted_res2, struct_resource{ -1 });

    n = 0;
    deleted_res1 = struct_resource{ -1 };
    deleted_res2 = struct_resource{ -1 };
    {
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur1{ res1, checking_resource_deleter< struct_resource >(deleted_res1, n) };
        BOOST_TEST_EQ(&ur1.get(), &res1);
        BOOST_TEST(ur1.allocated());
        struct_resource expected_res2{ 20 };
        boost::scope::unique_resource< struct_resource&, checking_resource_deleter< struct_resource > > ur2{ res2, checking_resource_deleter< struct_resource >(deleted_res2, n) };
        BOOST_TEST_EQ(&ur2.get(), &res2);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(n, 0);
        BOOST_TEST_EQ(&ur1.get(), &res2);
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(&ur2.get(), &res1);
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur2.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, struct_resource{ 10 });
    BOOST_TEST_EQ(deleted_res2, struct_resource{ 20 });
}


class throw_on_move_resource
{
private:
    int value;

public:
    explicit throw_on_move_resource(int v = 0) noexcept :
        value(v)
    {
    }

    throw_on_move_resource(throw_on_move_resource const&) = default;
    throw_on_move_resource& operator= (throw_on_move_resource const&) = default;

    throw_on_move_resource(throw_on_move_resource&&)
    {
        throw std::runtime_error("throw_on_move_resource move ctor");
    }
    throw_on_move_resource& operator= (throw_on_move_resource&&)
    {
        throw std::runtime_error("throw_on_move_resource move assignment");
    }

    int get() const noexcept
    {
        return value;
    }

    bool operator== (throw_on_move_resource const& that) const noexcept
    {
        return value == that.value;
    }

    bool operator!= (throw_on_move_resource const& that) const noexcept
    {
        return !operator==(that);
    }

    friend std::ostream& operator<< (std::ostream& strm, throw_on_move_resource const& res)
    {
        strm << "{ " << res.value << " }";
        return strm;
    }
};

class throw_on_copy_resource
{
private:
    int value;

public:
    explicit throw_on_copy_resource(int v = 0) noexcept :
        value(v)
    {
    }

    throw_on_copy_resource(throw_on_copy_resource const&)
    {
        throw std::runtime_error("throw_on_copy_resource copy ctor");
    }
    throw_on_copy_resource& operator= (throw_on_copy_resource const&)
    {
        throw std::runtime_error("throw_on_copy_resource copy assignment");
    }

    throw_on_copy_resource(throw_on_copy_resource&&) = delete;
    throw_on_copy_resource& operator= (throw_on_copy_resource&&) = delete;

    int get() const noexcept
    {
        return value;
    }

    bool operator== (throw_on_copy_resource const& that) const noexcept
    {
        return value == that.value;
    }

    bool operator!= (throw_on_copy_resource const& that) const noexcept
    {
        return !operator==(that);
    }

    friend std::ostream& operator<< (std::ostream& strm, throw_on_copy_resource const& res)
    {
        strm << "{ " << res.value << " }";
        return strm;
    }
};

void check_throw()
{
    int n = 0;
    try
    {
        boost::scope::unique_resource< throw_on_move_resource, checking_resource_deleter< throw_on_move_resource > > ur{ throw_on_move_resource{ 10 }, checking_resource_deleter< throw_on_move_resource >(n) };
        BOOST_TEST_EQ(ur.get().get(), 10);
        BOOST_TEST(ur.allocated());
    }
    catch (...)
    {
        BOOST_ERROR("An exception is not expected to be thrown by throw_on_move_resource (copy ctor should be used)");
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    try
    {
        boost::scope::unique_resource< throw_on_copy_resource, checking_resource_deleter< throw_on_copy_resource > > ur{ throw_on_copy_resource{ 10 }, checking_resource_deleter< throw_on_copy_resource >(n) };
        BOOST_ERROR("An exception is expected to be thrown by throw_on_copy_resource");
    }
    catch (...)
    {
    }
    BOOST_TEST_EQ(n, 1);
}


void check_deduction()
{
#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
    {
        typedef boost::scope::unique_resource< int, empty_resource_deleter< int > > expected_unique_resource_t;
        boost::scope::unique_resource ur{ 0, empty_resource_deleter< int >() };
        BOOST_TEST_TRAIT_SAME(decltype(ur), expected_unique_resource_t);
    }
    {
        typedef boost::scope::unique_resource< struct_resource, empty_resource_deleter< struct_resource > > expected_unique_resource_t;
        boost::scope::unique_resource ur{ struct_resource(), empty_resource_deleter< struct_resource >() };
        BOOST_TEST_TRAIT_SAME(decltype(ur), expected_unique_resource_t);
    }
    {
        typedef boost::scope::unique_resource< int, empty_resource_deleter< int > > expected_unique_resource_t;
        boost::scope::unique_resource ur1{ 0, empty_resource_deleter< int >() };
        BOOST_TEST_TRAIT_SAME(decltype(ur1), expected_unique_resource_t);
        boost::scope::unique_resource ur2 = std::move(ur1);
        BOOST_TEST_TRAIT_SAME(decltype(ur2), expected_unique_resource_t);
    }
#endif // !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)

    int n = 0, deleted_res = -1;
    {
        typedef boost::scope::unique_resource< int, checking_resource_deleter< int > > expected_unique_resource_t;
        auto ur = boost::scope::make_unique_resource_checked(10, 0, checking_resource_deleter< int >(deleted_res, n));
        BOOST_TEST_TRAIT_SAME(decltype(ur), expected_unique_resource_t);
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res, 10);

    n = 0;
    deleted_res = -1;
    {
        typedef boost::scope::unique_resource< int, checking_resource_deleter< int > > expected_unique_resource_t;
        auto ur = boost::scope::make_unique_resource_checked(0, 0, checking_resource_deleter< int >(deleted_res, n));
        BOOST_TEST_TRAIT_SAME(decltype(ur), expected_unique_resource_t);
        BOOST_TEST_EQ(ur.get(), 0);
        BOOST_TEST(!ur.allocated());
    }
    BOOST_TEST_EQ(n, 0);
    BOOST_TEST_EQ(deleted_res, -1);

    n = 0;
    try
    {
        auto ur = boost::scope::make_unique_resource_checked(throw_on_copy_resource{ 0 }, throw_on_copy_resource{ 0 }, checking_resource_deleter< throw_on_copy_resource >(n));
        BOOST_ERROR("An exception is expected to be thrown by throw_on_copy_resource");
    }
    catch (...)
    {
    }
    BOOST_TEST_EQ(n, 0);
}

struct int_resource_traits
{
    static int make_default() noexcept
    {
        return -1;
    }

    static bool is_allocated(int res) noexcept
    {
        return res >= 0;
    }
};

void check_resource_traits()
{
    {
        boost::scope::unique_resource< int, empty_resource_deleter< int >, int_resource_traits > ur;
        BOOST_TEST_EQ(ur.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur.allocated());
    }

    int n = 0, deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur{ -10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), -10);
        BOOST_TEST(!ur.allocated());
    }
    BOOST_TEST_EQ(n, 0);
    BOOST_TEST_EQ(deleted_res1, -1);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur{ 0, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 0);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, 0);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.release();
        BOOST_TEST_EQ(ur.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur.allocated());
    }
    BOOST_TEST_EQ(n, 0);
    BOOST_TEST_EQ(deleted_res1, -1);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.reset();
        BOOST_TEST(!ur.allocated());
        BOOST_TEST_EQ(ur.get(), int_resource_traits::make_default());
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, 10);
    }
    BOOST_TEST_EQ(n, 1);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur.get(), 10);
        BOOST_TEST(ur.allocated());
        ur.reset(20);
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, 10);
        deleted_res1 = -1;
        BOOST_TEST_EQ(ur.get(), 20);
        BOOST_TEST(ur.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 20);

    n = 0;
    deleted_res1 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur2{ std::move(ur1) };
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST_EQ(ur1.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur1.allocated());
    }
    BOOST_TEST_EQ(n, 1);
    BOOST_TEST_EQ(deleted_res1, 10);

    n = 0;
    deleted_res1 = -1;
    int deleted_res2 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur2{ 20, checking_resource_deleter< int >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST(ur2.allocated());
        ur2 = std::move(ur1);
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        BOOST_TEST_EQ(ur1.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur1.allocated());
        BOOST_TEST_EQ(n, 1);
        BOOST_TEST_EQ(deleted_res1, -1);
        BOOST_TEST_EQ(deleted_res2, 20);
        deleted_res2 = -1;
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 10);
    BOOST_TEST_EQ(deleted_res2, -1);

    {
        boost::scope::unique_resource< int, empty_resource_deleter< int >, int_resource_traits > ur1;
        BOOST_TEST_EQ(ur1.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur1.allocated());
        boost::scope::unique_resource< int, empty_resource_deleter< int >, int_resource_traits > ur2{ 10, empty_resource_deleter< int >() };
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), int_resource_traits::make_default());
        BOOST_TEST(!ur2.allocated());
    }

    n = 0;
    deleted_res1 = -1;
    deleted_res2 = -1;
    {
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur1{ 10, checking_resource_deleter< int >(deleted_res1, n) };
        BOOST_TEST_EQ(ur1.get(), 10);
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur1.allocated());
        boost::scope::unique_resource< int, checking_resource_deleter< int >, int_resource_traits > ur2{ 20, checking_resource_deleter< int >(deleted_res2, n) };
        BOOST_TEST_EQ(ur2.get(), 20);
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur2.allocated());
        using namespace std;
        swap(ur1, ur2);
        BOOST_TEST_EQ(n, 0);
        BOOST_TEST_EQ(ur1.get(), 20);
        BOOST_TEST_EQ(ur1.get_deleter().get_deleted(), &deleted_res2);
        BOOST_TEST(ur1.allocated());
        BOOST_TEST_EQ(ur2.get(), 10);
        BOOST_TEST_EQ(ur2.get_deleter().get_deleted(), &deleted_res1);
        BOOST_TEST(ur2.allocated());
    }
    BOOST_TEST_EQ(n, 2);
    BOOST_TEST_EQ(deleted_res1, 10);
    BOOST_TEST_EQ(deleted_res2, 20);
}

int main()
{
    check_int();
    check_struct();
    check_ptr();
    check_ref();
    check_throw();
    check_deduction();
    check_resource_traits();

    return boost::report_errors();
}
