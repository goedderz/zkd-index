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

set(GTEST_PATH vendor/rocksdb/third-party/gtest-1.8.1/fused-src/gtest)

add_library(my_gtest INTERFACE)
target_link_libraries(my_gtest INTERFACE gtest)
target_include_directories(my_gtest INTERFACE ${GTEST_PATH})

add_library(zkd_tree src/library.cpp src/library.h)
target_include_directories(zkd_tree INTERFACE src/)
#target_link_libraries(zkd_tree with_asan)

add_executable(zkd_tree_test test.cpp src/rocksdb-handle.cpp src/rocksdb-handle.h tests/zkd_test.cpp)
target_link_libraries(zkd_tree_test zkd_tree)
target_link_libraries(zkd_tree_test my_gtest)

target_link_libraries(zkd_tree_test my_rocksdb)
#target_link_libraries(zkd_tree_test with_asan)
