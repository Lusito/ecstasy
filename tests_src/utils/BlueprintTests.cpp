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

#define NS_TEST_CASE(name) TEST_CASE("Blueprint: " name)
namespace BlueprintTests {

	NS_TEST_CASE("test_component_blueprint_getters_default") {
		ComponentBlueprint blueprint("test");
		REQUIRE(blueprint.getBool("undefined", false) == false);
		REQUIRE(blueprint.getBool("undefined", true) == true);
		REQUIRE(blueprint.getInt("undefined", 42) == 42);
		REQUIRE(blueprint.getFloat("undefined", 3.14f) == 3.14f);
		REQUIRE(blueprint.getString("undefined", "paranoid android") == "paranoid android");
	}

	NS_TEST_CASE("test_component_blueprint_getters_invalid") {
		ComponentBlueprint blueprint("test");
		blueprint.set("bool", "");
		REQUIRE(blueprint.getBool("bool", true) == true);
		REQUIRE(blueprint.getBool("bool", false) == false);
		blueprint.set("bool", "1");
		REQUIRE(blueprint.getBool("bool", true) == true);
		REQUIRE(blueprint.getBool("bool", false) == false);

		blueprint.set("int", "");
		REQUIRE(blueprint.getInt("int", 42) == 42);
		blueprint.set("int", "invalid");
		REQUIRE(blueprint.getInt("int", 42) == 42);

		blueprint.set("float", "");
		REQUIRE(blueprint.getFloat("float", 42) == 42);
		blueprint.set("float", "invalid");
		REQUIRE(blueprint.getFloat("float", 42) == 42);
	}

	NS_TEST_CASE("test_component_blueprint_getters") {
		ComponentBlueprint blueprint("test");
		blueprint.set("bool", "true");
		REQUIRE(blueprint.getBool("bool", false) == true);
		blueprint.set("bool", "false");
		REQUIRE(blueprint.getBool("bool", true) == false);

		blueprint.set("int", "0");
		REQUIRE(blueprint.getInt("int", 42) == 0);
		blueprint.set("int", "012345");
		REQUIRE(blueprint.getInt("int", 42) == 12345);
		blueprint.set("int", "12345");
		REQUIRE(blueprint.getInt("int", 42) == 12345);
		blueprint.set("int", "-12345");
		REQUIRE(blueprint.getInt("int", 42) == -12345);

		blueprint.set("float", "0");
		REQUIRE(blueprint.getFloat("float", 42) == 0);
		blueprint.set("float", "0.12345");
		REQUIRE(blueprint.getFloat("float", 42) == 0.12345f);
		blueprint.set("float", "1.2345");
		REQUIRE(blueprint.getFloat("float", 42) == 1.2345f);
		blueprint.set("float", "-1.2345");
		REQUIRE(blueprint.getFloat("float", 42) == -1.2345f);

		blueprint.set("string", "hello world");
		REQUIRE(blueprint.getString("string", "foo bar") == "hello world");
	}

}
