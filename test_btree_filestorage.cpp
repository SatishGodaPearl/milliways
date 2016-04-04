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
#include "BTreeFileStorage.h"

#define B_TEST      4
#define BLOCK_SIZE  4096

TEST_CASE( "BTree File Storage", "[BTreeFileStorage]" ) {
	typedef milliways::BTree<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_t;
	typedef milliways::BTreeMemoryStorage<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_mem_st_t;
	typedef milliways::BTreeFileStorage< BLOCK_SIZE, B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_fs_t;
	typedef XTYPENAME btree_fs_t::block_storage_t btree_blockstorage_t;
	typedef XTYPENAME btree_t::node_type btree_node_t;
	typedef milliways::shptr<XTYPENAME btree_t::node_type> btree_node_ptr_t;
	typedef milliways::node_id_t btree_node_id_t;

	SECTION( "can be plugged in" )
	{
		btree_t tree;

		btree_fs_t* storage = new btree_fs_t("./test_tree");
		storage->attach(&tree);

		REQUIRE(tree.storage());
		REQUIRE(tree.storage() == storage);
		tree.close();
		assert(tree.storage() != NULL);
		storage->detach();
		assert(tree.storage() == NULL);
		delete storage;
	}

	SECTION( "can serialize/deserialize correct header" ) {
		btree_node_id_t root_id;

		const std::string test_pathname("./test_tree");

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			btree_t tree;

			std::remove(test_pathname.c_str());

			btree_fs_t* storage = new btree_fs_t(test_pathname);
			storage->attach(&tree);

			tree.open();
			REQUIRE(tree.isOpen());

			if (!tree.hasRoot())
				tree.node_alloc();
			btree_node_ptr_t root = tree.root();

			root_id = root->id();
			tree.close();
			storage->detach();
			delete storage;
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			btree_t tree;

			btree_fs_t* storage = new btree_fs_t(test_pathname);
			storage->attach(&tree);

			tree.open();
			REQUIRE(tree.isOpen());

			REQUIRE(tree.rootId() == root_id);
			tree.close();
			storage->detach();
			delete storage;
		}

		std::remove(test_pathname.c_str());
	}

	SECTION( "can serialize/deserialize a node" ) {
		btree_node_id_t root_id;

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			btree_t tree;

			btree_fs_t* storage = new btree_fs_t("./test_tree_2");
			storage->attach(&tree);

			tree.open();
			REQUIRE(tree.isOpen());

			if (!tree.hasRoot())
				tree.node_alloc();
			btree_node_ptr_t root = tree.root();

			btree_node_ptr_t child1 = root->child_alloc();
			btree_node_ptr_t child2 = root->child_alloc();
			root->n(1);
			root->child(0) = child1->id();
			root->child(1) = child2->id();
			root->key(0) = "Simple";

			child1->n(1);
			child1->key(0) = "first";
			child1->value(0) = 123;

			child2->n(1);
			child2->key(0) = "second";
			child2->value(0) = 575;

			root_id = root->id();

			// storage->node_put(root);
			// storage->node_put(child1);
			// storage->node_put(child2);

			tree.close();
			storage->detach();
			REQUIRE(! storage->isOpen());
			delete storage;
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			btree_t tree;

			btree_fs_t* storage = new btree_fs_t("./test_tree_2");
			storage->attach(&tree);

			tree.open();
			REQUIRE(tree.isOpen());

			REQUIRE(tree.rootId() == root_id);

			btree_node_ptr_t root = storage->node_get(tree.rootId());

			REQUIRE(root);
			REQUIRE(root->n() == 1);
			std::cerr << "root n:" << root->n() << " leaf:" << (root->leaf() ? "T" : "F") << " left:" << root->leftId() << " right:" << root->rightId() << "\n";

			tree.close();
			storage->detach();
			delete storage;
		}
	}

	SECTION( "can be attached to existing block storage" ) {
		btree_node_id_t root_id;

		const std::string test_pathname("./test_tree");

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			btree_t tree;

			std::remove(test_pathname.c_str());

			btree_blockstorage_t* bs = new btree_blockstorage_t(test_pathname);
			REQUIRE(bs);
			REQUIRE(! bs->isOpen());

			btree_fs_t* storage = new btree_fs_t(bs);
			REQUIRE(storage);
			REQUIRE(! bs->isOpen());
			REQUIRE(! storage->isOpen());
			REQUIRE(! tree.isOpen());

			storage->attach(&tree);

			tree.open();
			REQUIRE(bs->isOpen());
			REQUIRE(storage->isOpen());
			REQUIRE(tree.isOpen());

			btree_node_ptr_t root = tree.root();

			root_id = root->id();
			REQUIRE(milliways::node_id_valid(root_id));

			tree.close();
			REQUIRE(! bs->isOpen());
			REQUIRE(! storage->isOpen());
			REQUIRE(! tree.isOpen());
			storage->detach();
			delete bs;
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			btree_t tree;

			btree_blockstorage_t* bs = new btree_blockstorage_t(test_pathname);
			REQUIRE(bs);
			REQUIRE(! bs->isOpen());

			btree_fs_t* storage = new btree_fs_t(bs);
			REQUIRE(storage);
			REQUIRE(! bs->isOpen());
			REQUIRE(! storage->isOpen());
			REQUIRE(! tree.isOpen());

			storage->attach(&tree);

			tree.open();
			REQUIRE(bs->isOpen());
			REQUIRE(storage->isOpen());
			REQUIRE(tree.isOpen());

			REQUIRE(tree.rootId() == root_id);

			btree_node_ptr_t root = tree.root();

			REQUIRE(root->id() == root_id);

			tree.close();
			REQUIRE(! bs->isOpen());
			REQUIRE(! storage->isOpen());
			REQUIRE(! tree.isOpen());
			storage->detach();
			delete bs;
		}

		std::remove(test_pathname.c_str());
	}
}
