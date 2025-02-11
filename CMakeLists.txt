cmake_minimum_required(VERSION 3.28)
project(voidtools)

set(CMAKE_CXX_STANDARD 23)

add_executable(voidtools main.cpp
        win_exports.cpp
        win_exports.hpp
        exports.cpp
        exports.hpp)

if(WIN32)
    # Winsock2
    target_link_libraries(voidtools PUBLIC wsock32 ws2_32)
endif()

# cxxopts
target_include_directories(voidtools PRIVATE vendors/cxxopts-3.2.0/include)

# lua
add_subdirectory(vendors/lua-5.4.7)
target_include_directories(voidtools PRIVATE vendors/lua-5.4.7/src)
target_link_libraries(voidtools PRIVATE lua54)
target_include_directories(voidtools PRIVATE vendors/sol2-3.3.0/single/single/include)

# fmt
add_subdirectory(vendors/fmt-11.1.3)
target_link_libraries(voidtools PRIVATE fmt::fmt)

# rini
target_include_directories(voidtools PRIVATE vendors/rini-main/src)