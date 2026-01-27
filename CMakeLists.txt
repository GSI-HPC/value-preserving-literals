# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
#                       Matthias Kretz <m.kretz@gsi.de>

cmake_minimum_required(VERSION 3.30)
project(value-preserving-literals
        VERSION 0.0.1
        DESCRIPTION "value-preserving literals for C++"
        HOMEPAGE_URL "https://github.com/GSI-HPC/value-preserving-literals"
        LANGUAGES CXX)

# Define main library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Require at least C++26 with reflection
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add reflection support for GCC
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-freflection FLAG_REFLECTION)
if (FLAG_REFLECTION)
  add_compile_options(-freflection)
endif()

set(MSVC_WARNINGS "")
set(CLANG_WARNINGS "-Wall -Wextra")
set(GCC_WARNINGS "${CLANG_WARNINGS} -Wconversion -Wsign-conversion")

if(MSVC)
  set(CMAKE_CXX_FLAGS ${MSVC_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  set(CMAKE_CXX_FLAGS ${CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS ${GCC_WARNINGS})
endif()

# Define the install target
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}
        NAMESPACE gsi::
        DESTINATION lib/cmake/gsi)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Define test targets
enable_testing()

# Add test for arithmetic.cpp
add_executable(arithmetic_test tests/arithmetic.cpp)
target_link_libraries(arithmetic_test PRIVATE value-preserving-literals)

# Add to test suite
add_test(NAME arithmetic COMMAND arithmetic_test)
