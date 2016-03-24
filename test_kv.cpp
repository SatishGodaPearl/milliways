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

#include "KeyValueStore.h"

static inline int rand_int(int lo, int hi)
{
	return rand() % (hi - lo + 1) + lo;
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

TEST_CASE( "KeyValue store", "[KeyValueStore]" ) {
	typedef milliways::KeyValueStore kv_t;
	typedef typename kv_t::block_storage_type kv_blockstorage_t;

	SECTION( "freshly created kv store works" ) {
		const std::string test_pathname("/tmp/test_kv");

		REQUIRE(true);

		std::remove(test_pathname.c_str());

		kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

		kv_t kv(bs);

		kv.open();
		REQUIRE(kv.isOpen());

		REQUIRE(! kv.has("foo"));
		REQUIRE(! kv.has("Mickey"));

		kv.put("foo", "bar");
		REQUIRE(kv.has("foo"));

		kv.put("Mickey", "Mouse");
		REQUIRE(kv.has("Mickey"));

		std::string value;
		REQUIRE(kv.get("foo", value));
		REQUIRE(value == "bar");

		REQUIRE(kv.get("Mickey", value));
		REQUIRE(value == "Mouse");

		REQUIRE(! kv.get("Boston", value));

		kv.close();
	}

	SECTION( "load after write/close works" ) {
		const std::string test_pathname("/tmp/test_kv");

		REQUIRE(true);

		std::remove(test_pathname.c_str());

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			REQUIRE(! kv.has("foo"));
			REQUIRE(! kv.has("Mickey"));

			kv.put("foo", "bar");
			REQUIRE(kv.has("foo"));

			kv.put("Mickey", "Mouse");
			REQUIRE(kv.has("Mickey"));

			std::string value;
			REQUIRE(kv.get("foo", value));
			REQUIRE(value == "bar");

			REQUIRE(kv.get("Mickey", value));
			REQUIRE(value == "Mouse");

			REQUIRE(! kv.get("Boston", value));

			kv.close();
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			REQUIRE(kv.has("foo"));
			REQUIRE(kv.has("Mickey"));

			std::string value;
			REQUIRE(kv.get("foo", value));
			REQUIRE(value == "bar");

			REQUIRE(kv.get("Mickey", value));
			REQUIRE(value == "Mouse");

			REQUIRE(! kv.get("Boston", value));

			kv.close();
		}

		std::remove(test_pathname.c_str());
	}

	SECTION( "iteration works" ) {
		const std::string test_pathname("/tmp/test_kv");

		std::remove(test_pathname.c_str());

		kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

		kv_t kv(bs);

		kv.open();
		REQUIRE(kv.isOpen());

		REQUIRE(! kv.has("foo"));
		REQUIRE(! kv.has("Mickey"));

		kv.put("foo", "bar");
		kv.put("Mickey", "Mouse");
		kv.put("color", "blue");
		kv.put("z", "534");
		kv.put("height", "928m");
		kv.put("aaa", "12");

		kv_t::iterator it_end(kv.end());
		REQUIRE(it_end.end());

		kv_t::iterator it = kv.begin();
		REQUIRE(! it.end());

		REQUIRE(it != it_end);
		REQUIRE((*it) == "Mickey");
		std::cerr << it << std::endl;

		++it;
		REQUIRE(! it.end());
		REQUIRE(it != it_end);
		REQUIRE((*it) == "aaa");

		++it;
		REQUIRE(! it.end());
		REQUIRE(it != it_end);
		REQUIRE((*it) == "color");

		++it;
		REQUIRE(! it.end());
		REQUIRE(it != it_end);
		REQUIRE((*it) == "foo");

		++it;
		REQUIRE(! it.end());
		REQUIRE(it != it_end);
		REQUIRE((*it) == "height");

		++it;
		REQUIRE(! it.end());
		REQUIRE(it != it_end);
		REQUIRE((*it) == "z");

		++it;
		REQUIRE(it.end());
		REQUIRE(it == it_end);
		std::cerr << it << std::endl;

		for (kv_t::iterator it = kv.begin(); it != kv.end(); ++it)
		{
			std::cerr << (*it) << std::endl;
		}

		kv.close();
	}

	SECTION( "straming reads" ) {
		const std::string test_pathname("/tmp/test_kv");

		std::remove(test_pathname.c_str());

		kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

		kv_t kv(bs);

		kv.open();
		REQUIRE(kv.isOpen());

		kv.put("foo", "bar");
		kv.put("Mickey", "Mouse");
		kv.put("color", "blue");
		kv.put("z", "534");
		kv.put("height", "928m");
		kv.put("aaa", "12");
		kv.put("longish", "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

		kv_t::Search search;
		kv.find("longish", search);
		REQUIRE(search.found());
		REQUIRE(search.valid());

		std::string piece;
		bool ok = kv.get(search, piece, 6);
		REQUIRE(ok);
		std::cerr << piece;
		REQUIRE(piece == "Lorem ");

		ok = kv.get(search, piece, 6);
		REQUIRE(ok);
		std::cerr << piece;
		REQUIRE(piece == "ipsum ");

		ok = kv.get(search, piece, 6);
		REQUIRE(ok);
		std::cerr << piece;
		REQUIRE(piece == "dolor ");

		ok = kv.get(search, piece, 4);
		REQUIRE(ok);
		std::cerr << piece;
		REQUIRE(piece == "sit ");

		ok = kv.get(search, piece);
		REQUIRE(ok);
		std::cerr << piece << std::endl;
		REQUIRE(piece.substr(0,5) == "amet,");
		REQUIRE(piece.length() > 8);
		REQUIRE(piece.substr(piece.length()-8,8) == "laborum.");

		kv.close();
	}

	SECTION( "largish sets works" ) {
		const std::string test_pathname("/tmp/test_kv");

		std::remove(test_pathname.c_str());

		typedef std::map<std::string, std::string> kv_set_t;
		kv_set_t test_set;
		const int test_set_size = 2048;
		const int max_key_len = 20;
		const int max_value_len = 256;

		for (int i = 0; i < test_set_size; ++i)
		{
			std::string key = random_string(rand_int(1, max_key_len));
			std::string value = random_string(rand_int(1, max_value_len));
			test_set[key] = value;
		}

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
				REQUIRE(kv.put(t_it->first, t_it->second));

			for (kv_t::iterator it = kv.begin(); it != kv.end(); ++it)
			{
				std::string key = (*it);
				REQUIRE(test_set.count(key) == 1);
				std::string value;
				REQUIRE(kv.get(*it, value));
				REQUIRE(value == test_set[key]);
			}

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			{
				const std::string& key = t_it->first;
				const std::string& value = t_it->second;

				REQUIRE(kv.has(key));
				std::string kv_value;
				REQUIRE(kv.get(key, kv_value));
				REQUIRE(kv_value == value);
			}

			kv.close();
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			for (kv_t::iterator it = kv.begin(); it != kv.end(); ++it)
			{
				std::string key = (*it);
				REQUIRE(test_set.count(key) == 1);
				std::string value;
				REQUIRE(kv.get(*it, value));
				REQUIRE(value == test_set[key]);
			}

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			{
				const std::string& key = t_it->first;
				const std::string& value = t_it->second;

				REQUIRE(kv.has(key));
				std::string kv_value;
				REQUIRE(kv.get(key, kv_value));
				REQUIRE(kv_value == value);
			}

			kv.close();
		}

		std::remove(test_pathname.c_str());
	}

	SECTION( "largish values works" ) {
		const std::string test_pathname("/tmp/test_kv");

		std::remove(test_pathname.c_str());

		const std::string large_value1 = random_string(8192);
		const std::string large_value2 = random_string(8192);

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			kv.put("foo", large_value1);
			REQUIRE(kv.has("foo"));

			kv.put("bar", large_value2);
			REQUIRE(kv.has("bar"));

			std::string value;
			REQUIRE(kv.get("foo", value));
			REQUIRE(value == large_value1);
			REQUIRE(kv.get("bar", value));
			REQUIRE(value == large_value2);

			kv.close();
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			REQUIRE(kv.has("foo"));
			REQUIRE(kv.has("bar"));

			std::string value;
			REQUIRE(kv.get("foo", value));
			REQUIRE(value == large_value1);
			REQUIRE(kv.get("bar", value));
			REQUIRE(value == large_value2);

			kv.close();
		}

		std::remove(test_pathname.c_str());
	}

	SECTION( "sets with largish values works" ) {
		const std::string test_pathname("/tmp/test_kv");

		std::remove(test_pathname.c_str());

		typedef std::map<std::string, std::string> kv_set_t;
		kv_set_t test_set;
		const int test_set_size = 16;
		const int max_key_len = 20;
		const int min_value_len = 4096;
		const int max_value_len = 16384;

		for (int i = 0; i < test_set_size; ++i)
		{
			std::string key = random_string(rand_int(1, max_key_len));
			std::string value = random_string(rand_int(min_value_len, max_value_len));
			test_set[key] = value;
		}

		std::cerr << "CREATE AND WRITE" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
				REQUIRE(kv.put(t_it->first, t_it->second));

			for (kv_t::iterator it = kv.begin(); it != kv.end(); ++it)
			{
				std::string key = (*it);
				REQUIRE(test_set.count(key) == 1);
				std::string value;
				REQUIRE(kv.get(*it, value));
				REQUIRE(value == test_set[key]);
			}

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			{
				const std::string& key = t_it->first;
				const std::string& value = t_it->second;

				REQUIRE(kv.has(key));
				std::string kv_value;
				REQUIRE(kv.get(key, kv_value));
				REQUIRE(kv_value == value);
			}

			kv.close();
		}

		std::cerr << "OPEN AND LOAD" << std::endl;
		{
			kv_blockstorage_t* bs = new kv_blockstorage_t(test_pathname);

			kv_t kv(bs);

			kv.open();
			REQUIRE(kv.isOpen());

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
				REQUIRE(kv.put(t_it->first, t_it->second));

			for (kv_t::iterator it = kv.begin(); it != kv.end(); ++it)
			{
				std::string key = (*it);
				REQUIRE(test_set.count(key) == 1);
				std::string value;
				REQUIRE(kv.get(*it, value));
				REQUIRE(value == test_set[key]);
			}

			for (kv_set_t::const_iterator t_it = test_set.begin(); t_it != test_set.end(); ++t_it)
			{
				const std::string& key = t_it->first;
				const std::string& value = t_it->second;

				REQUIRE(kv.has(key));
				std::string kv_value;
				REQUIRE(kv.get(key, kv_value));
				REQUIRE(kv_value == value);
			}

			kv.close();
		}

		std::remove(test_pathname.c_str());
	}
}
