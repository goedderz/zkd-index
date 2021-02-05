cmake_minimum_required(VERSION 3.17)
project(zkd_tree)

set(CMAKE_CXX_STANDARD 17)

set(ROCKSDB_BUILD_SHARED OFF CACHE BOOL "build shared libraries")
set(WITH_TOOLS OFF CACHE BOOL "disable tools")
set(WITH_CORE_TOOLS OFF CACHE BOOL "disable core tools")
set(WITH_TESTS OFF CACHE BOOL "disable tests")
set(WITH_GFLAGS 0 CACHE BOOL "")
add_subdirectory(vendor/rocksdb)

add_library(my_rocksdb INTERFACE)
target_link_libraries(my_rocksdb INTERFACE rocksdb)
target_include_directories(my_rocksdb INTERFACE vendor/rocksdb/include)


set(BuildVelocyPackExamples OFF CACHE BOOL "")
set(Maintainer OFF CACHE BOOL "")
add_subdirectory(vendor/velocypack)

set(immer_BUILD_TESTS OFF CACHE BOOL "")
set(immer_BUILD_EXAMPLES OFF CACHE BOOL "")
set(immer_BUILD_DOCS OFF CACHE BOOL "")
set(immer_BUILD_EXTRAS OFF CACHE BOOL "")
add_subdirectory(vendor/immer EXCLUDE_FROM_ALL)

add_library(with_asan INTERFACE)
target_compile_options(with_asan INTERFACE "-fsanitize=address")
target_link_libraries(with_asan INTERFACE asan)



add_library(zkd_tree library.cpp library.h)
target_link_libraries(zkd_tree with_asan)

add_executable(zkd_tree_test main.cpp rocksdb-handle.cpp rocksdb-handle.h)
target_link_libraries(zkd_tree_test zkd_tree)

target_link_libraries(zkd_tree_test my_rocksdb)
target_link_libraries(zkd_tree_test with_asan)
