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
#include <ecstasy/utils/Blueprint.hpp>
#include <ecstasy/utils/BlueprintParser.hpp>

#define NS_TEST_CASE(name) TEST_CASE("BlueprintParser: " name)
namespace BlueprintParserTests {

	NS_TEST_CASE("test_good_file") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/good.def", blueprint);
		REQUIRE(error.empty());
	}

	NS_TEST_CASE("test_bad_command") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_command.def", blueprint);
		REQUIRE(error == "Line 1: unknown command 'whoops'");
	}

	NS_TEST_CASE("test_bad_insufficient_parameters") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_insufficient_parameters.def", blueprint);
		REQUIRE(error == "Line 1: expected exactly one argument to 'add'");
	}

	NS_TEST_CASE("test_bad_too_many_parameters") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_too_many_parameters.def", blueprint);
		REQUIRE(error == "Line 1: expected exactly one argument to 'add'");
	}

	NS_TEST_CASE("test_bad_insufficient_parameters2") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_insufficient_parameters2.def", blueprint);
		REQUIRE(error == "Line 2: expected exactly two arguments to 'set'");
	}

	NS_TEST_CASE("test_bad_too_many_parameters2") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_too_many_parameters2.def", blueprint);
		REQUIRE(error == "Line 2: expected exactly two arguments to 'set'");
	}

	NS_TEST_CASE("test_bad_open_quote") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_open_quote.def", blueprint);
		REQUIRE(error == "Line 2: quote has not been closed");
	}

	NS_TEST_CASE("test_bad_wrong_order") {
		std::shared_ptr<EntityBlueprint> blueprint;
		auto error = parseBlueprint("tests_assets/bad_wrong_order.def", blueprint);
		REQUIRE(error == "Line 1: 'add' must be called before 'set'");
	}
}
