# Copyright 2023 Andrey Semashev
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.20)

project(boost_scope VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_scope INTERFACE)
add_library(Boost::scope ALIAS boost_scope)

target_include_directories(boost_scope INTERFACE include)

target_link_libraries(boost_scope
    INTERFACE
        Boost::config
        Boost::core
        Boost::type_traits
)

if (BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
    add_subdirectory(test)
endif()
