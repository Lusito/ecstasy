/*******************************************************************************
 * Copyright 2015 See AUTHORS file.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include "../TestBase.hpp"

namespace BitsTests {

	TEST_CASE("test_most_significant_bits_hashcode_and_equals") {
		Bits b1;
		Bits b2;

		b1.set(1);
		b2.set(1);

		REQUIRE(b1.getStringId() == b2.getStringId());
		REQUIRE(b1 == b2);

		// temporarily setting/clearing a single bit causing
		// the backing array to grow
		b2.set(420);
		b2.clear(420);

		REQUIRE(b1.getStringId() ==  b2.getStringId());
		REQUIRE(b1 == b2);

		b1.set(810);
		b1.clear(810);

		REQUIRE(b1.getStringId() == b2.getStringId());
		REQUIRE(b1 == b2);
	}

	TEST_CASE("test_xor") {
		Bits b1;
		Bits b2;

		b2.set(200);

		// b1:s array should grow to accommodate b2
		b1 ^= b2;

		REQUIRE(b1.get(200));

		b1.set(1024);
		b2 ^= b1;

		REQUIRE(b2.get(1024));
	}

	TEST_CASE("test_or") {
		Bits b1;
		Bits b2;

		b2.set(200);

		// b1:s array should grow to accommodate b2
		b1 |= b2;

		REQUIRE(b1.get(200));

		b1.set(1024);
		b2 |= b1;

		REQUIRE(b2.get(1024));
	}

	TEST_CASE("test_and") {
		Bits b1;
		Bits b2;

		b2.set(200);
		// b1 should cancel b2:s bit
		b2 &= b1;

		REQUIRE(!b2.get(200));

		b1.set(400);
		b1 &= b2;

		REQUIRE(!b1.get(400));
	}
}
