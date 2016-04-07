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

#include <string>
#include <vector>
#include <utility>

#include "LRUCache.h"

#define LRU_SIZE    4

#define INVALID_INT_KEY		-1000000

bool startsWith(const std::string& s, const std::string& prefix)
{
	return (s.size() >= prefix.size()) && (s.substr(0, prefix.size()) == prefix);
}

bool endsWith(const std::string& s, const std::string& suffix)
{
	return (s.size() >= suffix.size()) && (s.substr(s.size() - suffix.size(), suffix.size()) == suffix);
}

bool contains(const std::string& s, const std::string& infix)
{
	return s.find(infix) != std::string::npos;
}

TEST_CASE( "LRU Cache", "[LRUCache]" ) {
	typedef milliways::LRUCache<LRU_SIZE, int, std::string> lru_t;
	lru_t lru(INVALID_INT_KEY);

	REQUIRE(lru.size() == 0);
	REQUIRE(lru.max_size() == LRU_SIZE);

	SECTION( "starts empty" ) {
		REQUIRE(lru.size() == 0);
		REQUIRE(lru.empty());
	}

	SECTION( "can add elements" ) {
		REQUIRE(lru.size() == 0);

		lru[128] = "128";
		lru[24] = "24";

		REQUIRE(lru.size() == 2);
	}

	SECTION( "can retrieve elements" ) {
		REQUIRE(lru.size() == 0);

		lru[128] = "128";
		lru[24] = "24";
		lru[256] = "256";

		REQUIRE(lru.size() == 3);

		REQUIRE(lru[24] == "24");
		REQUIRE(lru[256] == "256");
		REQUIRE(lru[128] == "128");
	}

	SECTION( "can pop LRU item" ) {
		REQUIRE(lru.size() == 0);

		lru[128] = "128";
		lru[24] = "24";
		lru[256] = "256";

		REQUIRE(lru.size() == 3);

		lru_t::value_type item;

		item = lru.pop();
		REQUIRE(item.first == 128);

		item = lru.pop();
		REQUIRE(item.first == 24);

		item = lru.pop();
		REQUIRE(item.first == 256);

		REQUIRE(lru.size() == 0);
	}

	SECTION( "pushing over size butts LRU item" ) {
		REQUIRE(lru.size() == 0);

		lru[128] = "128";
		lru[24] = "24";
		lru[256] = "256";
		lru[15] = "15";
		lru[30] = "30";

		REQUIRE(lru.size() == LRU_SIZE);

		lru_t::value_type item;

		item = lru.pop();
		REQUIRE(item.first == 24);

		item = lru.pop();
		REQUIRE(item.first == 256);

		item = lru.pop();
		REQUIRE(item.first == 15);

		item = lru.pop();
		REQUIRE(item.first == 30);

		REQUIRE(lru.size() == 0);
	}
}

static std::string miss_string(int value) {
	std::ostringstream ss;
	ss << "M-" << static_cast<int>(value);
	return ss.str();
}

static std::string hit_string(int value) {
	std::ostringstream ss;
	ss << "hit-" << static_cast<int>(value);
	return ss.str();
}

class ExtLRUCache : public milliways::LRUCache<LRU_SIZE, int, std::string>
{
public:
	typedef milliways::LRUCache<LRU_SIZE, int, std::string> base_type;
	typedef int key_type;
	typedef std::string mapped_type;

	ExtLRUCache() :
		milliways::LRUCache<LRU_SIZE, int, std::string>(INVALID_INT_KEY),
		m_n_miss(0), m_n_miss_get(0), m_n_miss_set(0), m_n_miss_sub(0),
		m_n_set(0), m_n_eviction(0)
		{ }
	virtual ~ExtLRUCache() { evict_all(); }

	long n_miss() const { return m_n_miss; }
	long n_miss_get() const { return m_n_miss_get; }
	long n_miss_set() const { return m_n_miss_set; }
	long n_miss_sub() const { return m_n_miss_sub; }
	long n_set() const { return m_n_set; }
	long n_eviction() const { return m_n_eviction; }

	bool on_miss(XTYPENAME base_type::op_type op, const key_type& key, mapped_type& value)
	{
		m_n_miss++;
		switch (op)
		{
		case base_type::op_get:
			m_n_miss_get++;
			value = miss_string(key);
			break;
		case base_type::op_set:
			m_n_miss_set++;
			break;
		case base_type::op_sub:
			m_n_miss_sub++;
			value = miss_string(key);
			break;
		}
		return true;
	}
	bool on_set(const key_type& key, const mapped_type& value)
	{
		m_n_set++;
		return true;
	}
	//bool on_delete(const key_type& key);
	bool on_eviction(const key_type& key, mapped_type& value)
	{
		m_n_eviction++;
		return true;
	}

private:
	long m_n_miss;
	long m_n_miss_get;
	long m_n_miss_set;
	long m_n_miss_sub;
	long m_n_set;
	long m_n_eviction;
};

TEST_CASE( "Extended LRU Cache", "[ExtendedLRUCache]" ) {
	static const int TEST_SET_SIZE = 20;

	SECTION("adding values") {
		ExtLRUCache lru;

		for (int i = 0; i < TEST_SET_SIZE; i++)
			lru[i] = hit_string(i);
	}

	SECTION("adding and checking values") {
		ExtLRUCache lru;

		REQUIRE(lru.size() == 0);
		for (int i = 0; i < TEST_SET_SIZE; i++)
		{
			lru[i] = hit_string(i);
			REQUIRE(lru[i] == hit_string(i));
		}
		REQUIRE(lru.size() == 4);
		REQUIRE(lru.n_miss_sub() > 0);
		REQUIRE(lru.n_eviction() > 0);
	}

	SECTION("hit and miss") {
		ExtLRUCache lru;

		REQUIRE(lru.size() == 0);
		for (int i = 0; i < TEST_SET_SIZE; i++)
			lru[i] = hit_string(i);

		REQUIRE(lru.n_miss_sub() == TEST_SET_SIZE);
		REQUIRE(lru.n_eviction() == TEST_SET_SIZE - 4);
		std::cerr << "lru[8373]:" << lru[8373] << std::endl;
	}
}
