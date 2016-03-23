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

#include "ordered_map.h"

TEST_CASE( "Ordered map", "[ordered_map]" ) {
	milliways::ordered_map<std::string, int> omap;

	REQUIRE(omap.size() == 0);

	SECTION( "can add elements" ) {
		omap["Zed"] = 1;
		omap["Something"] = 2;
		omap["Abc"] = 3;

		REQUIRE(omap.size() == 3);

		REQUIRE(omap.size() == 3);
	}

	SECTION( "can find elements" ) {
		REQUIRE(omap.size() == 0);

		omap["Zed"] = 1;
		omap["Something"] = 2;
		omap["Abc"] = 3;

		REQUIRE(omap.size() == 3);

		REQUIRE(omap["Zed"] == 1);
		REQUIRE(omap["Something"] == 2);
		REQUIRE(omap["Abc"] == 3);
	}

	SECTION( "iteration respects insertion order" ) {
		omap["Zed"] = 1;
		omap["Something"] = 2;
		omap["Abc"] = 3;

		REQUIRE(omap.size() == 3);

		milliways::ordered_map<std::string, int>::iterator it;

		it = omap.begin();
		REQUIRE(it);

		REQUIRE(it->first == "Zed");
		REQUIRE(it->second == 1);
		REQUIRE(it != omap.end());
		++it;
		REQUIRE(it);

		REQUIRE(it->first == "Something");
		REQUIRE(it->second == 2);
		REQUIRE(it != omap.end());
		++it;

		REQUIRE(it->first == "Abc");
		REQUIRE(it->second == 3);
		REQUIRE(it != omap.end());
		++it;

		REQUIRE(it == omap.end());
		REQUIRE(!it);
	}

	SECTION( "const iterator works too" ) {
		omap["Zed"] = 1;
		omap["Something"] = 2;
		omap["Abc"] = 3;

		REQUIRE(omap.size() == 3);

		milliways::ordered_map<std::string, int>::const_iterator it;

		it = omap.begin();
		REQUIRE(it);

		REQUIRE(it->first == "Zed");
		REQUIRE(it->second == 1);
		REQUIRE(it != omap.end());
		++it;
		REQUIRE(it);

		REQUIRE(it->first == "Something");
		REQUIRE(it->second == 2);
		REQUIRE(it != omap.end());
		++it;

		REQUIRE(it->first == "Abc");
		REQUIRE(it->second == 3);
		REQUIRE(it != omap.end());
		++it;

		REQUIRE(it == omap.end());
		REQUIRE(!it);
	}

	SECTION( "pop() produces items in reverse order" ) {
		omap["Zed"] = 1;
		omap["Something"] = 2;
		omap["Abc"] = 3;

		REQUIRE(omap.size() == 3);

		milliways::ordered_map<std::string, int>::value_type item;

		item = omap.pop();
		REQUIRE(item.first == "Abc");
		REQUIRE(item.second == 3);

		item = omap.pop();
		REQUIRE(item.first == "Something");
		REQUIRE(item.second == 2);

		item = omap.pop();
		REQUIRE(item.first == "Zed");
		REQUIRE(item.second == 1);

		REQUIRE(omap.size() == 0);
	}
}