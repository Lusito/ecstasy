/*******************************************************************************
 * Copyright 2014 See AUTHORS file.
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
#include "../TestBase.h"
#include <ecstasy/systems/IteratingSystem.h>

namespace FamilyTests {
	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};
	struct ComponentC : public Component<ComponentC> {};
	struct ComponentD : public Component<ComponentD> {};
	struct ComponentE : public Component<ComponentE> {};
	struct ComponentF : public Component<ComponentF> {};

	class TestSystemA : public IteratingSystem<TestSystemA> {
	public:
		TestSystemA(std::string name) : IteratingSystem(Family::all<ComponentA>().get()) {}

	protected:
		void processEntity(Entity *e, float d) override {}
	};

	class TestSystemB : public IteratingSystem<TestSystemB> {
	public:
		TestSystemB (std::string name) : IteratingSystem(Family::all<ComponentB>().get()) {}

		void processEntity (Entity *e, float d) override {}
	};

	TEST_CASE("sameFamily") {
		auto &family1 = Family::all<ComponentA>().get();
		auto &family2 = Family::all<ComponentA>().get();
		auto &family3 = Family::all<ComponentA, ComponentB>().get();
		auto &family4 = Family::all<ComponentA, ComponentB>().get();
		auto &family5 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		auto &family6 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		auto &family7 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		auto &family8 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		auto &family9 = Family::all().get();
		auto &family10 = Family::all().get();

		REQUIRE(family1 == family2);
		REQUIRE(family2 == family1);
		REQUIRE(family3 == family4);
		REQUIRE(family4 == family3);
		REQUIRE(family5 == family6);
		REQUIRE(family6 == family5);
		REQUIRE(family7 == family8);
		REQUIRE(family8 == family7);
		REQUIRE(family9 == family10);

		REQUIRE(family1.index == family2.index);
		REQUIRE(family3.index == family4.index);
		REQUIRE(family5.index == family6.index);
		REQUIRE(family7.index == family8.index);
		REQUIRE(family9.index == family10.index);
	}

	TEST_CASE("differentFamily") {
		auto &family1 = Family::all<ComponentA>().get();
		auto &family2 = Family::all<ComponentB>().get();
		auto &family3 = Family::all<ComponentC>().get();
		auto &family4 = Family::all<ComponentA, ComponentB>().get();
		auto &family5 = Family::all<ComponentA, ComponentC>().get();
		auto &family6 = Family::all<ComponentB, ComponentA>().get();
		auto &family7 = Family::all<ComponentB, ComponentC>().get();
		auto &family8 = Family::all<ComponentC, ComponentA>().get();
		auto &family9 = Family::all<ComponentC, ComponentB>().get();
		auto &family10 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		auto &family11 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		auto &family12 = Family::all<ComponentC, ComponentD>().one<ComponentE, ComponentF>()
			.exclude<ComponentA, ComponentB>().get();
		auto &family13 = Family::all().get();

		REQUIRE(family1 != family2);
		REQUIRE(family1 != family3);
		REQUIRE(family1 != family4);
		REQUIRE(family1 != family5);
		REQUIRE(family1 != family6);
		REQUIRE(family1 != family7);
		REQUIRE(family1 != family8);
		REQUIRE(family1 != family9);
		REQUIRE(family1 != family10);
		REQUIRE(family1 != family11);
		REQUIRE(family1 != family12);
		REQUIRE(family1 != family13);

		REQUIRE(family10 != family1);
		REQUIRE(family10 != family2);
		REQUIRE(family10 != family3);
		REQUIRE(family10 != family4);
		REQUIRE(family10 != family5);
		REQUIRE(family10 != family6);
		REQUIRE(family10 != family7);
		REQUIRE(family10 != family8);
		REQUIRE(family10 != family9);
		REQUIRE(family11 != family12);
		REQUIRE(family10 != family13);

		REQUIRE(family1.index != family2.index);
		REQUIRE(family1.index != family3.index);
		REQUIRE(family1.index != family4.index);
		REQUIRE(family1.index != family5.index);
		REQUIRE(family1.index != family6.index);
		REQUIRE(family1.index != family7.index);
		REQUIRE(family1.index != family8.index);
		REQUIRE(family1.index != family9.index);
		REQUIRE(family1.index != family10.index);
		REQUIRE(family11.index != family12.index);
		REQUIRE(family1.index != family13.index);
	}

	TEST_CASE("familyEqualityFiltering") {
		auto &family1 = Family::all<ComponentA>().one<ComponentB>().exclude<ComponentC>().get();
		auto &family2 = Family::all<ComponentB>().one<ComponentC>().exclude<ComponentA>().get();
		auto &family3 = Family::all<ComponentC>().one<ComponentA>().exclude<ComponentB>().get();
		auto &family4 = Family::all<ComponentA>().one<ComponentB>().exclude<ComponentC>().get();
		auto &family5 = Family::all<ComponentB>().one<ComponentC>().exclude<ComponentA>().get();
		auto &family6 = Family::all<ComponentC>().one<ComponentA>().exclude<ComponentB>().get();

		REQUIRE(family1 == family4);
		REQUIRE(family2 == family5);
		REQUIRE(family3 == family6);
		REQUIRE(family1 != family2);
		REQUIRE(family1 != family3);
	}

	TEST_CASE("entityMatch") {
		auto &family = Family::all<ComponentA, ComponentB>().get();

		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		entity->add(engine.createComponent<ComponentA>());
		entity->add(engine.createComponent<ComponentB>());

		REQUIRE(family.matches(entity));

		entity->add(engine.createComponent<ComponentC>());

		REQUIRE(family.matches(entity));
	}

	TEST_CASE("entityMismatch") {
		auto &family = Family::all<ComponentA, ComponentC>().get();

		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		ComponentA *a = engine.createComponent<ComponentA>();
		ComponentB *b = engine.createComponent<ComponentB>();
		entity->add(a);
		entity->add(b);

		REQUIRE(!family.matches(entity));

		entity->remove<ComponentB>();

		REQUIRE(!family.matches(entity));
	}

	TEST_CASE("entityMatchThenMismatch") {
		auto &family = Family::all<ComponentA, ComponentB>().get();

		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		entity->add(engine.createComponent<ComponentA>());
		entity->add(engine.createComponent<ComponentB>());

		REQUIRE(family.matches(entity));

		entity->remove<ComponentA>();

		REQUIRE(!family.matches(entity));
	}

	TEST_CASE("entityMismatchThenMatch") {
		auto &family = Family::all<ComponentA, ComponentB>().get();

		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		entity->add(engine.createComponent<ComponentA>());
		entity->add(engine.createComponent<ComponentC>());

		REQUIRE(!family.matches(entity));

		entity->add(engine.createComponent<ComponentB>());

		REQUIRE(family.matches(entity));
	}

	TEST_CASE("testEmptyFamily") {
		auto &family = Family::all().get();
		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		REQUIRE(family.matches(entity));
	}

	TEST_CASE("familyFiltering") {
		auto &family1 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();

		auto &family2 = Family::all<ComponentC, ComponentD>().one<ComponentA, ComponentB>()
			.exclude<ComponentE, ComponentF>().get();

		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);

		REQUIRE(!family1.matches(entity));
		REQUIRE(!family2.matches(entity));

		entity->add(engine.createComponent<ComponentA>());
		entity->add(engine.createComponent<ComponentB>());

		REQUIRE(!family1.matches(entity));
		REQUIRE(!family2.matches(entity));

		entity->add(engine.createComponent<ComponentC>());

		REQUIRE(family1.matches(entity));
		REQUIRE(!family2.matches(entity));

		entity->add(engine.createComponent<ComponentD>());

		REQUIRE(family1.matches(entity));
		REQUIRE(family2.matches(entity));

		entity->add(engine.createComponent<ComponentE>());

		REQUIRE(!family1.matches(entity));
		REQUIRE(!family2.matches(entity));

		entity->remove<ComponentE>();

		REQUIRE(family1.matches(entity));
		REQUIRE(family2.matches(entity));

		entity->remove<ComponentA>();

		REQUIRE(!family1.matches(entity));
		REQUIRE(family2.matches(entity));
	}

	TEST_CASE("matchWithEngine") {
		Engine engine;
		TestSystemA systemA("A");
		TestSystemB systemB("B");
		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		auto *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		auto &f = Family::all<ComponentB>().exclude<ComponentA>().get();

		REQUIRE(!f.matches(e));
	}

	TEST_CASE("matchWithEngineInverse") {
		Engine engine;

		TestSystemA systemA("A");
		TestSystemB systemB("B");
		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		auto *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		auto &f = Family::all<ComponentA>().exclude<ComponentB>().get();

		REQUIRE(!f.matches(e));
	}

	TEST_CASE("matchWithoutSystems") {
		Engine engine;

		auto *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		auto &f = Family::all<ComponentB>().exclude<ComponentA>().get();

		REQUIRE(!f.matches(e));
	}

	TEST_CASE("matchWithComplexBuilding") {
		auto &family = Family::all<ComponentB>().one<ComponentA>().exclude<ComponentC>().get();
		Engine engine;
		Entity *entity = engine.createEntity();
		engine.addEntity(entity);
		entity->add(engine.createComponent<ComponentA>());
		REQUIRE(!family.matches(entity));
		entity->add(engine.createComponent<ComponentB>());
		REQUIRE(family.matches(entity));
		entity->add(engine.createComponent<ComponentC>());
		REQUIRE(!family.matches(entity));
	}
}
