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

#include <map>
#include <unordered_map>

#include "hashtable.h"

static inline int rand_int(int lo, int hi)
{
	return rand() % (hi - lo + 1) + lo;
}

static inline uint32_t rand_uint32(uint32_t lo, uint32_t hi)
{
	return static_cast<uint32_t>(rand()) % (hi - lo + 1) + lo;
}

static std::string random_string(int length)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string r(length, '\0');

	for (int i = 0; i < length; ++i)
		r[i] = alphanum[rand_int(0, sizeof(alphanum) - 1)];

	return r;
}

TEST_CASE( "Hash Table", "[hashtable]" ) {
	typedef milliways::hashtable<std::string, int> ht_t;
	typedef std::unordered_map<std::string, int> test_set_t;

	test_set_t test_set;
	const int test_set_size = 2048;
	const int max_key_len = 10;
	const int max_value = 1000;

	for (int i = 0; i < test_set_size; ++i)
	{
		std::string key = random_string(rand_int(1, max_key_len));
		int value = rand_int(1, max_value);
		test_set[key] = value;
	}

	ht_t table(test_set_size);
	REQUIRE(table.empty());

	SECTION( "can add elements" ) {
		table.set("Zed", 1);
		table.set("Something", 2);
		table.set("Abc", 3);

		std::cerr << table << std::endl;
	}

	SECTION( "set & erase & get" ) {
		table.set("Zed", 1);
		table.set("Something", 2);
		table.set("Abc", 3);

		REQUIRE(table.has("Zed"));
		REQUIRE(table.has("Something"));
		REQUIRE(table.has("Abc"));

		table.erase("Abc");
		REQUIRE(table.has("Zed"));
		REQUIRE(table.has("Something"));
		REQUIRE(! table.has("Abc"));
		REQUIRE(table["Zed"] == 1);
		REQUIRE(table["Something"] == 2);

		table.set("Abc", 4);
		REQUIRE(table.has("Zed"));
		REQUIRE(table.has("Something"));
		REQUIRE(table.has("Abc"));
		REQUIRE(table["Abc"] == 4);
		REQUIRE(table["Zed"] == 1);
		REQUIRE(table["Something"] == 2);

		table.erase("Something");
		REQUIRE(table.has("Zed"));
		REQUIRE(! table.has("Something"));
		REQUIRE(table.has("Abc"));
		REQUIRE(table["Abc"] == 4);
		REQUIRE(table["Zed"] == 1);

		table.set("Something", 10);
		REQUIRE(table["Abc"] == 4);
		REQUIRE(table["Zed"] == 1);
		REQUIRE(table["Something"] == 10);

		std::cerr << table << std::endl;
	}

	SECTION( "many elements - set() & has()" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			REQUIRE(table.has(t_it->first));
	}

	SECTION( "many elements - set & get()" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it) {
			int value = 0;
			REQUIRE(table.get(t_it->first, value));
			REQUIRE(value == t_it->second);
		}
	}

	SECTION( "iteration works" ) {
		table.set("Zed", 1);
		table.set("Something", 2);
		table.set("Abc", 3);

		std::cerr << table << std::endl;

		milliways::hashtable<std::string, int>::iterator it;

		it = table.begin();
		REQUIRE(it);

		REQUIRE(it != table.end());
		++it;
		REQUIRE(it);

		REQUIRE(it != table.end());
		++it;

		REQUIRE(it != table.end());
		++it;

		REQUIRE(it == table.end());
		REQUIRE(!it);
	}

	SECTION( "many elements - set & iteration" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		int n = 0;
		milliways::hashtable<std::string, int>::iterator it;
		for (it = table.begin(); it != table.end(); ++it) {
			std::string key = it->first;
			int value = it->second;
			REQUIRE(test_set[key] == value);
			n++;
		}
		REQUIRE(n == test_set.size());
	}
}

TEST_CASE( "Hash Table uint32_t", "[hashtable_u32]" ) {
	typedef milliways::hashtable<uint32_t, uint32_t> ht_t;
	typedef milliways::hashtable<uint32_t, uint32_t>::iterator ht_it_t;
	typedef std::unordered_map<uint32_t, uint32_t> test_set_t;

	test_set_t test_set;
	const long test_set_size = 131072;
	const long max_value = 1000000;

	for (long i = 0; i < test_set_size; ++i)
	{
		uint32_t key = rand_uint32(0, max_value);
		uint32_t value = rand_uint32(0, max_value);
		test_set[key] = value;
	}

	ht_t table(test_set_size / 3);
	REQUIRE(table.empty());

	SECTION( "many elements - set() & has()" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			REQUIRE(table.has(t_it->first));
	}

	SECTION( "many elements - set & get()" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it) {
			uint32_t value = 0;
			REQUIRE(table.get(t_it->first, value));
			REQUIRE(value == t_it->second);
		}
	}

	SECTION( "many elements - set & iteration" ) {
		for (test_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			table.set(t_it->first, t_it->second);

		long n = 0;
		for (ht_it_t it = table.begin(); it != table.end(); ++it) {
			uint32_t key = it->first;
			uint32_t value = it->second;
			REQUIRE(test_set[key] == value);
			n++;
		}
		REQUIRE(n == test_set.size());
	}
}
