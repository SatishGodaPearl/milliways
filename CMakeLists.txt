cmake_minimum_required(VERSION 3.3)
project(milliways)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

set(SOURCE_FILES test_shptr.cpp catch.hpp Utils.h Utils.impl.hpp)
add_executable(test_shptr ${SOURCE_FILES})
