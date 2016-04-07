/*****************************************************************************/
/*  Milliways - B+ trees and key-value store C++ library                     */
/*                                                                           */
/*  Copyright 2016 Marco Pantaleoni and J CUBE Inc. Tokyo, Japan.            */
/*                                                                           */
/*  Author: Marco Pantaleoni <marco.pantaleoni@gmail.com>                    */
/*                                                                           */
/*  Licensed under the Apache License, Version 2.0 (the "License");          */
/*  you may not use this file except in compliance with the License.         */
/*  You may obtain a copy of the License at                                  */
/*                                                                           */
/*      http://www.apache.org/licenses/LICENSE-2.0                           */
/*                                                                           */
/*  Unless required by applicable law or agreed to in writing, software      */
/*  distributed under the License is distributed on an "AS IS" BASIS,        */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*  See the License for the specific language governing permissions and      */
/*  limitations under the License.                                           */
/*****************************************************************************/

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Seriously.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "BTreeFileStorage.h"

#define B_TEST      4
#define BLOCK_SIZE  4096

TEST_CASE( "BTree Operations", "[BTreeOps]" ) {
	typedef milliways::BTree<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_t;
	typedef milliways::BTreeMemoryStorage<B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_mem_st_t;
	typedef milliways::BTreeFileStorage< BLOCK_SIZE, B_TEST, seriously::Traits<std::string>, seriously::Traits<int32_t> > btree_fs_t;
	typedef XTYPENAME btree_t::node_type btree_node_t;
	typedef milliways::shptr<XTYPENAME btree_t::node_type> btree_node_ptr_t;
	typedef XTYPENAME btree_t::lookup_type btree_lookup_t;
	typedef milliways::node_id_t btree_node_id_t;

	SECTION( "can search in a node" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		if (!tree.hasRoot())
			tree.node_alloc();
		btree_node_ptr_t root = tree.root();

		btree_node_ptr_t child1 = root->child_alloc();
		btree_node_ptr_t child2 = root->child_alloc();
		root->n(1);
		root->child(0) = child1->id();
		root->child(1) = child2->id();
		root->key(0) = "c";

		child1->n(3);
		child1->key(0) = "abc";
		child1->value(0) = 123;
		child1->key(1) = "about";
		child1->value(1) = 874;
		child1->key(2) = "antiparticle";
		child1->value(2) = 268;

		child2->n(1);
		child2->key(0) = "def";
		child2->value(0) = 575;

		root_id = root->id();

		btree_lookup_t lookup;
		bool found = child1->search(lookup, "about");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.pos() == 1);
		REQUIRE(lookup.key() == "about");

		found = child1->search(lookup, "abridged");
		REQUIRE(! found);
		REQUIRE(! lookup.found());
		REQUIRE(lookup.pos() == 2);
		REQUIRE(lookup.key() == "abridged");

		found = root->search(lookup, "def");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.pos() == 0);
		REQUIRE(lookup.key() == "def");

		tree.close();
	}

	SECTION( "can insert" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		tree.insert("13", 13);
		tree.insert("7", 7);
		tree.insert("23", 23);
		tree.insert("31", 31);
		tree.insert("43", 43);
		tree.insert("11", 11);
		tree.insert("2", 2);
		tree.insert("17", 17);
		tree.insert("5", 5);
		tree.insert("3", 3);

		btree_lookup_t lookup;
		bool found = tree.search(lookup, "13");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "13");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "7");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "7");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "23");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "23");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "31");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "31");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "43");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "43");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "11");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "11");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "2");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "2");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "17");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "17");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "5");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "5");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "3");
		REQUIRE(found);
		REQUIRE(lookup.found());
		REQUIRE(lookup.key() == "3");
		std::cerr << lookup << std::endl;

		found = tree.search(lookup, "foo");
		REQUIRE(! found);
		REQUIRE(! lookup.found());
		REQUIRE(lookup.key() == "foo");
		std::cerr << lookup << std::endl;

		tree.dotGraph("./tree-dot", /* display */ true);

		tree.close();
	}

	SECTION( "can iterate forward" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		tree.insert("13", 13);
		tree.insert("7", 7);
		tree.insert("23", 23);
		tree.insert("31", 31);
		tree.insert("43", 43);
		tree.insert("11", 11);
		tree.insert("2", 2);
		tree.insert("17", 17);
		tree.insert("5", 5);
		tree.insert("3", 3);

		typedef XTYPENAME btree_t::iterator btree_iterator_t;
		btree_iterator_t it = tree.begin();

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "11");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "13");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "17");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "2");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "23");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "3");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "31");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "43");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "5");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "7");
		it++;

		REQUIRE(! it);
		REQUIRE(! ((*it).found()));

		for (btree_iterator_t it = tree.begin(); it != tree.end(); ++it)
		{
			btree_t::lookup_type& l = *it;

			REQUIRE(l.found());
			REQUIRE(l.node());
			btree_t::key_type key = l.key();
			btree_t::mapped_type val = l.node()->value(l.pos());
			std::cerr << l << std::endl;
		}

		tree.close();
	}

	SECTION( "can iterate backward" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		tree.insert("13", 13);
		tree.insert("7", 7);
		tree.insert("23", 23);
		tree.insert("31", 31);
		tree.insert("43", 43);
		tree.insert("11", 11);
		tree.insert("2", 2);
		tree.insert("17", 17);
		tree.insert("5", 5);
		tree.insert("3", 3);

		typedef XTYPENAME btree_t::iterator btree_iterator_t;
		btree_iterator_t it = tree.rbegin();

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "7");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "5");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "43");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "31");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "3");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "23");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "2");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "17");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "13");
		it++;

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "11");
		it++;

		REQUIRE(! it);
		REQUIRE(! ((*it).found()));

		for (btree_iterator_t it = tree.rbegin(); it != tree.rend(); ++it)
		{
			btree_t::lookup_type& l = *it;

			REQUIRE(l.found());
			REQUIRE(l.node());
			btree_t::key_type key = l.key();
			btree_t::mapped_type val = l.node()->value(l.pos());
			std::cerr << l << std::endl;
		}

		tree.close();
	}

	SECTION( "can iterate forward and backward simultaneously" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		tree.insert("13", 13);
		tree.insert("7", 7);
		tree.insert("23", 23);
		tree.insert("31", 31);
		tree.insert("43", 43);
		tree.insert("11", 11);
		tree.insert("2", 2);
		tree.insert("17", 17);
		tree.insert("5", 5);
		tree.insert("3", 3);

		typedef XTYPENAME btree_t::iterator btree_iterator_t;
		btree_iterator_t it = tree.begin();

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "11");

		it++;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "13");

		it++;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "17");

		it--;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "13");

		it--;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "11");

		it--;
		REQUIRE(! it);
		REQUIRE(! ((*it).found()));

		tree.close();
	}

	SECTION( "can iterate forward and backward simultaneously also for a reverse iterator" ) {
		btree_node_id_t root_id;

		btree_t tree;

//		btree_fs_t* storage = new btree_fs_t("/tmp/test_tree_2");
//		storage->attach(&tree);
//		tree.open();

		tree.insert("13", 13);
		tree.insert("7", 7);
		tree.insert("23", 23);
		tree.insert("31", 31);
		tree.insert("43", 43);
		tree.insert("11", 11);
		tree.insert("2", 2);
		tree.insert("17", 17);
		tree.insert("5", 5);
		tree.insert("3", 3);

		typedef XTYPENAME btree_t::iterator btree_iterator_t;
		btree_iterator_t it = tree.rbegin();

		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "7");

		it++;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "5");

		it++;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "43");

		it--;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "5");

		it--;
		REQUIRE(it);
		REQUIRE((*it).found());
		REQUIRE((*it).key() == "7");

		it--;
		REQUIRE(! it);
		REQUIRE(! ((*it).found()));

		tree.close();
	}
}
