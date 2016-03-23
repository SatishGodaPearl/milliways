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

#include "LRUCache.h"

#define LRU_SIZE    4

TEST_CASE( "LRU Cache", "[LRUCache]" ) {
	typedef milliways::LRUCache<LRU_SIZE, int, std::string> lru_t;
	lru_t lru;

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
