/*
 * milliways - B+ trees and key-value store C++ library
 *
 * Author: Marco Pantaleoni <marco.pantaleoni@gmail.com>
 * Copyright (C) 2016 Marco Pantaleoni. All rights reserved.
 *
 * Distributed under the Apache License, Version 2.0
 * See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 * The author licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Seriously.h"
#include "BTreeNode.h"
#include "BTree.h"

#define B_TEST  4

TEST_CASE( "BTree", "[BTree]" ) {
	typedef milliways::BTree<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_t;
	typedef XTYPENAME btree_t::node_type btree_node_t;
	typedef milliways::shptr<XTYPENAME btree_t::node_type> btree_node_ptr_t;

	SECTION( "starts empty" ) {
		btree_t tree;

		REQUIRE(tree.size() == 0);
		REQUIRE(! tree.hasRoot());
	}

	SECTION( "can alloc children" ) {
		btree_t tree;
		btree_node_ptr_t root = tree.node_alloc();

		REQUIRE(root);
		REQUIRE(tree.rootId() == root->id());

		btree_node_ptr_t child = tree.node_child_alloc(root);

		REQUIRE(child);
		REQUIRE(child->leaf());

		REQUIRE(child->id() != root->id());
		REQUIRE(child->parentId() == root->id());
		REQUIRE(! root->leaf());
		REQUIRE(tree.rootId() == root->id());
	}

	SECTION( "root is created automatically when requested" ) {
		btree_t tree;
		btree_node_ptr_t root = tree.root();

		REQUIRE(root);
		REQUIRE(tree.rootId() == root->id());
	}

	SECTION( "size and root properties behave correctly" ) {
		btree_t tree;

		REQUIRE(tree.size() == 0);
		REQUIRE(! tree.hasRoot());

		btree_node_ptr_t node = tree.node_alloc();

		REQUIRE(node);
		REQUIRE(node->leaf());
		REQUIRE(tree.size() == 1);
		REQUIRE(tree.hasRoot());
		REQUIRE(tree.rootId() == node->id());

		REQUIRE(node->id() != milliways::NODE_ID_INVALID);

		REQUIRE(node->empty());
		REQUIRE(node->nonFull());
	}
}

TEST_CASE( "BTree Memory Storage", "[BTreeMemoryStorage]" ) {
	typedef milliways::BTree<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_t;
	typedef milliways::BTreeMemoryStorage<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_mem_st_t;
	typedef XTYPENAME btree_t::node_type btree_node_t;
	typedef milliways::shptr<XTYPENAME btree_t::node_type> btree_node_ptr_t;

	SECTION( "is present by default" )
	{
		btree_t tree;

		REQUIRE(tree.storage());
	}

	SECTION( "can get requested nodes" ) {
		btree_t tree;

		if (! tree.hasRoot()) {
			tree.node_alloc();
		}
		btree_node_ptr_t root = tree.root();

		btree_node_ptr_t child1 = tree.node_child_alloc(root);
		btree_node_ptr_t child2 = tree.node_child_alloc(root);
		btree_node_ptr_t child3 = tree.node_child_alloc(root);

		btree_node_ptr_t child1_1 = tree.node_child_alloc(child1);
		btree_node_ptr_t child3_1 = tree.node_child_alloc(child3);
		btree_node_ptr_t child3_2 = tree.node_child_alloc(child3);

		btree_node_ptr_t node = tree.node_read(root->id());
		REQUIRE(node->id() == root->id());

		node = tree.node_read(child1->id());
		REQUIRE(node->id() == child1->id());

		node = tree.node_read(child2->id());
		REQUIRE(node->id() == child2->id());

		node = tree.node_read(child3->id());
		REQUIRE(node->id() == child3->id());

		node = tree.node_read(child1_1->id());
		REQUIRE(node->id() == child1_1->id());

		node = tree.node_read(child3_1->id());
		REQUIRE(node->id() == child3_1->id());

		node = tree.node_read(child3_2->id());
		REQUIRE(node->id() == child3_2->id());
	}
}
