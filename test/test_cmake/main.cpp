//  Copyright (c) 2023 Andrey Semashev
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  https://www.boost.org/LICENSE_1_0.txt)

#include <boost/scope/scope_exit.hpp>

struct exit_func
{
    void operator()() const noexcept
    {
    }
};

int main()
{
    boost::scope::scope_exit< exit_func > guard{ exit_func() };
    return 0;
}
