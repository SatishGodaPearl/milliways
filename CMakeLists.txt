cmake_minimum_required(VERSION 2.8)
project(milliways)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(CheckCXXSourceCompiles)
include(CheckTypeSize)
check_cxx_source_compiles("
    #include <memory>
    int main() { std::shared_ptr<int> p; return 0; }
" HAVE_STD_SHARED_PTR)
check_cxx_source_compiles("
    #include <tr1/memory>
    int main() { std::tr1::shared_ptr<int> p; return 0; }
" HAVE_STD_TR1_SHARED_PTR)
check_cxx_source_compiles("
    #include <boost/shared_ptr.hpp>
    int main() { boost::shared_ptr<int> p; return 0; }
" HAVE_BOOST_SHARED_PTR)

check_cxx_source_compiles("
	#include <stdint.h>
	#include <stddef.h>

	template <typename T>
	struct Traits;

	template <>
	struct Traits<uint8_t>
	{ typedef uint8_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint16_t>
	{ typedef uint16_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint32_t>
	{ typedef uint32_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint64_t>
	{ typedef uint64_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<size_t>
	{ typedef size_t type; static size_t size() { return sizeof(type); } };

	int main() { size_t size = Traits<size_t>::size(); size++; }
" ALLOWS_TEMPLATED_SIZE_T)

check_cxx_source_compiles("
	#include <stdint.h>
	#include <stddef.h>

	template <typename T>
	struct Traits;

	template <>
	struct Traits<uint8_t>
	{ typedef uint8_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint16_t>
	{ typedef uint16_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint32_t>
	{ typedef uint32_t type; static size_t size() { return sizeof(type); } };

	template <>
	struct Traits<uint64_t>
	{ typedef uint64_t type; static size_t size() { return sizeof(type); } };

	int main() { size_t size = Traits<size_t>::size(); size++; }
" DOESNT_NEED_TEMPLATED_SIZE_T)

SET(CMAKE_EXTRA_INCLUDE_FILES stdint.h)
CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T LANGUAGE CXX)
CHECK_TYPE_SIZE(ssize_t SIZEOF_SSIZE_T LANGUAGE CXX)
CHECK_TYPE_SIZE(short SIZEOF_SHORT LANGUAGE CXX)
CHECK_TYPE_SIZE(int SIZEOF_INT LANGUAGE CXX)
CHECK_TYPE_SIZE(long SIZEOF_LONG LANGUAGE CXX)
CHECK_TYPE_SIZE("unsigned long" SIZEOF_UNSIGNED_LONG LANGUAGE CXX)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG LANGUAGE CXX)
CHECK_TYPE_SIZE(float SIZEOF_FLOAT LANGUAGE CXX)
CHECK_TYPE_SIZE(double SIZEOF_DOUBLE LANGUAGE CXX)
SET(CMAKE_EXTRA_INCLUDE_FILES)

configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file (config.h.cmake ${PROJECT_SOURCE_DIR}/config.h )

#set(SOURCE_FILES main.cpp BlockStorage.impl.hpp BlockStorage.h LRUCache.impl.hpp LRUCache.h ordered_map.h ordered_map.impl.hpp)
#add_executable(milliways ${SOURCE_FILES})

set(SOURCE_FILES test_ordered_map.cpp catch.hpp ordered_map.h ordered_map.impl.hpp)
add_executable(test_ordered_map ${SOURCE_FILES})

set(SOURCE_FILES test_lrucache.cpp catch.hpp ordered_map.h ordered_map.impl.hpp LRUCache.h LRUCache.impl.hpp)
add_executable(test_lrucache ${SOURCE_FILES})

set(SOURCE_FILES test_btree_btreenode.cpp catch.hpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp)
add_executable(test_btree_btreenode ${SOURCE_FILES})

set(SOURCE_FILES test_btree_filestorage.cpp catch.hpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp)
add_executable(test_btree_filestorage ${SOURCE_FILES})

set(SOURCE_FILES test_btree_ops.cpp catch.hpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp)
add_executable(test_btree_ops ${SOURCE_FILES})

set(SOURCE_FILES test_kv.cpp catch.hpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp KeyValueStore.h KeyValueStore.impl.hpp)
add_executable(test_kv ${SOURCE_FILES})

set(SOURCE_FILES test_kv2.cpp catch.hpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp KeyValueStore.h KeyValueStore.impl.hpp)
add_executable(test_kv2 ${SOURCE_FILES})

set(SOURCE_FILES test_shptr.cpp catch.hpp Utils.h Utils.impl.hpp)
add_executable(test_shptr ${SOURCE_FILES})

set(SOURCE_FILES benchmark_kv.cpp Utils.h Utils.impl.hpp Seriously.h Seriously.impl.hpp BlockStorage.h BlockStorage.impl.hpp BTreeCommon.h BTreeNode.h BTreeNode.impl.hpp BTree.h BTree.impl.hpp BTreeFileStorage.h BTreeFileStorage.impl.hpp KeyValueStore.h KeyValueStore.impl.hpp)
add_executable(benchmark_kv ${SOURCE_FILES})
