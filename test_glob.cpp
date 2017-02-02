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

#include "config.h"

#include "glob.h"

TEST_CASE( "glob", "[glob]" ) {

	SECTION( "matches exact patterns" ) {
		REQUIRE(milliways::glob("exact string", "exact string"));
		REQUIRE(! milliways::glob("exact string", "different"));
		REQUIRE(! milliways::glob("exact string", "another string"));
		REQUIRE(! milliways::glob("exact string", "not an exact string"));
		REQUIRE(! milliways::glob("exact string", "exact string no"));
		REQUIRE(! milliways::glob("exact string", "no exact string no"));
	}

	SECTION( "matches * wildcard" ) {
		REQUIRE(milliways::glob("* string", "wild string"));
		REQUIRE(! milliways::glob("* string", "different string!"));
		REQUIRE(! milliways::glob("* string", "another strin"));

		REQUIRE(milliways::glob("also *", "also string"));
		REQUIRE(! milliways::glob("also *", "not also string!"));
		REQUIRE(! milliways::glob("also *", "lso string"));

		REQUIRE(milliways::glob("an * string", "an awesome string"));
		REQUIRE(milliways::glob("an * string", "an  string"));
		REQUIRE(! milliways::glob("an * string", "not an awesome string"));
		REQUIRE(! milliways::glob("an * string", "an awesome string not"));
		REQUIRE(! milliways::glob("an * string", "n uncool strin"));
	}

	SECTION( "matches ? wildcard" ) {
		REQUIRE(milliways::glob("?", "z"));
		REQUIRE(milliways::glob("that ?", "that q"));
		REQUIRE(milliways::glob("? also", "a also"));

		REQUIRE(! milliways::glob("?", ""));
		REQUIRE(! milliways::glob("?", "zz"));
		REQUIRE(! milliways::glob("that ?", "that qq"));
		REQUIRE(! milliways::glob("that ?", "that "));
		REQUIRE(! milliways::glob("? also", "aa also"));
		REQUIRE(! milliways::glob("? also", " also"));
	}

	SECTION( "matches [XYZ] and [X-Y] classes" ) {
		REQUIRE(milliways::glob("[kbd]", "b"));
		REQUIRE(milliways::glob("[kbd]", "k"));
		REQUIRE(milliways::glob("[kbd]", "d"));

		REQUIRE(! milliways::glob("[kbd]", "dd"));
		REQUIRE(! milliways::glob("[kbd]", "f"));

		REQUIRE(milliways::glob("[c-i]", "f"));
		REQUIRE(! milliways::glob("[c-i]", "b"));
		REQUIRE(! milliways::glob("[c-i]", "j"));

		REQUIRE(milliways::glob("[c-i0-9]", "f"));
		REQUIRE(milliways::glob("[c-i0-9]", "5"));
		REQUIRE(! milliways::glob("[c-i0-9]", "b"));
		REQUIRE(! milliways::glob("[c-i0-9]", "j"));
	}

	SECTION( "matches [!XYZ] and [!X-Y] classes" ) {
		REQUIRE(! milliways::glob("[!kbd]", "b"));
		REQUIRE(! milliways::glob("[!kbd]", "k"));
		REQUIRE(! milliways::glob("[!kbd]", "d"));

		REQUIRE(milliways::glob("[! kbd]", "K"));
		REQUIRE(milliways::glob("[! kbd]", "f"));

		REQUIRE(! milliways::glob("[!c-i]", "f"));
		REQUIRE(milliways::glob("[!c-i]", "b"));
		REQUIRE(milliways::glob("[!c-i]", "j"));

		REQUIRE(milliways::glob("[!c-i0-9]", "b"));
		REQUIRE(milliways::glob("[!c-i0-9]", "j"));
		REQUIRE(! milliways::glob("[!c-i0-9]", "f"));
		REQUIRE(! milliways::glob("[!c-i0-9]", "5"));
	}

}
