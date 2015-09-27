cmake_minimum_required(VERSION 3.0)
project(numcrunch)

#enable_testing()

include(CheckSymbolExists)
#check_symbol_exists(sys_signame signal.h HAVE_SIGNAME)

# set compiler flags
# string concat is easier in 3.0, but older versions don't have the concat subcommand
# so we are using list as input until we move to new version
# TODO once we add build types, we should also set flags such as "-O2 "
string(CONCAT LOCAL_CFLAGS
    "-std=c11 "
    "-ggdb3 "
    "-O3 -march=native "
    "-Wall -Wshadow -Winline "
    "-Wstrict-prototypes -Wmissing-prototypes "
    "-Wmissing-declarations -Wredundant-decls "
    "-Wunused-function -Wunused-value -Wunused-variable ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  ${LOCAL_CFLAGS}")

# build dependencies

#set(CMAKE_INCLUDE_CURRENT_DIR)
include_directories(${include_directories}
    "${PROJECT_SOURCE_DIR}/src")

add_subdirectory(src)

# print a summary
message(STATUS "CPPFLAGS: " ${CMAKE_CPP_FLAGS})
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})
