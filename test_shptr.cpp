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

#include "Utils.h"

class Counted
{
public:
	Counted() { s_count++; s_creations++; }
	virtual ~Counted() { s_count--; s_destructions++; }

	long callme() { return 42; }

	static long Existing() { return s_count; }
	static long Created() { return s_creations; }
	static long Destroyed() { return s_destructions; }

	static void Reset() { s_count = 0; s_creations = 0; s_destructions = 0; }

private:
	static long s_count;
	static long s_creations;
	static long s_destructions;
};

long Counted::s_count = 0;
long Counted::s_creations = 0;
long Counted::s_destructions = 0;

TEST_CASE( "shptr shared pointer", "[shptr]" ) {
	typedef milliways::shptr<Counted> counted_sptr_t;

	SECTION( "basics work as expected" ) {
		Counted* counted = new Counted();

		REQUIRE(Counted::Existing() == 1);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 0);

		{
			milliways::shptr<Counted> sptr1(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1);
			REQUIRE(sptr1.get() == counted);
			REQUIRE((void*)sptr1 != NULL);
			REQUIRE((void*)sptr1 == (void*)counted);
			REQUIRE(sptr1.count() == 1);

			REQUIRE(sptr1->callme() == 42);
			REQUIRE((*sptr1).callme() == 42);

			milliways::shptr<Counted> sptr2(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr2);
			REQUIRE(sptr2.get() == counted);
			REQUIRE((void*)sptr2 == (void*)counted);
			REQUIRE(sptr1.count() == 2);
			REQUIRE(sptr2.count() == 2);

			REQUIRE(sptr2 == sptr1);

			std::cerr << "shared ptr: " << sptr2 << std::endl;
		}
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 1);
	}

	SECTION( "copy-construction and assignment work too" ) {
		Counted::Reset();
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 0);
		REQUIRE(Counted::Destroyed() == 0);

		Counted* counted = new Counted();

		REQUIRE(Counted::Existing() == 1);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 0);

		{
			milliways::shptr<Counted> sptr1(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1.count() == 1);

			milliways::shptr<Counted> sptr2(sptr1);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1.count() == 2);
			REQUIRE(sptr2.count() == 2);

			std::cerr << "shared ptr: " << sptr2 << std::endl;

			milliways::shptr<Counted> sptr3 = sptr1;

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1.count() == 3);
			REQUIRE(sptr2.count() == 3);
			REQUIRE(sptr3.count() == 3);
		}
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 1);
	}

	SECTION( "reset works with one copy" ) {
		Counted::Reset();
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 0);
		REQUIRE(Counted::Destroyed() == 0);

		Counted* counted = new Counted();

		REQUIRE(Counted::Existing() == 1);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 0);

		{
			milliways::shptr<Counted> sptr1(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1);
			REQUIRE(sptr1.count() == 1);

			sptr1.reset();

			REQUIRE(! sptr1);
			REQUIRE((void*)sptr1 == NULL);
			REQUIRE(sptr1.count() == 0);

			REQUIRE(Counted::Existing() == 0);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 1);
		}
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 1);
	}

	SECTION( "reset works with multiple copies" ) {
		Counted::Reset();
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 0);
		REQUIRE(Counted::Destroyed() == 0);

		Counted* counted = new Counted();

		REQUIRE(Counted::Existing() == 1);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 0);

		{
			milliways::shptr<Counted> sptr1(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1);
			REQUIRE(sptr1.count() == 1);

			milliways::shptr<Counted> sptr2(counted);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);

			REQUIRE(sptr1.count() == 2);
			REQUIRE(sptr2.count() == 2);

			sptr1.reset();

			REQUIRE(! sptr1);
			REQUIRE((void*)sptr1 == NULL);
			REQUIRE(sptr1.count() == 0);

			REQUIRE(sptr2);
			REQUIRE(sptr2.get() == counted);
			REQUIRE(sptr2.count() == 1);

			REQUIRE(Counted::Existing() == 1);
			REQUIRE(Counted::Created() == 1);
			REQUIRE(Counted::Destroyed() == 0);
		}
		REQUIRE(Counted::Existing() == 0);
		REQUIRE(Counted::Created() == 1);
		REQUIRE(Counted::Destroyed() == 1);
	}
}
