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
	struct ComponentA : public Component<ComponentA>, public Poolable {
		void reset() override {}
	};
	struct ComponentB : public Component<ComponentB>, public Poolable {
		void reset() override {}
	};
	struct ComponentC : public Component<ComponentC>, public Poolable {
		void reset() override {}
	};
	struct ComponentD : public Component<ComponentD>, public Poolable {
		void reset() override {}
	};
	struct ComponentE : public Component<ComponentE>, public Poolable {
		void reset() override {}
	};
	struct ComponentF : public Component<ComponentF>, public Poolable {
		void reset() override {}
	};

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
		Family &family1 = Family::all<ComponentA>().get();
		Family &family2 = Family::all<ComponentA>().get();
		Family &family3 = Family::all<ComponentA, ComponentB>().get();
		Family &family4 = Family::all<ComponentA, ComponentB>().get();
		Family &family5 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		Family &family6 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		Family &family7 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		Family &family8 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		Family &family9 = Family::all().get();
		Family &family10 = Family::all().get();

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
		Family &family1 = Family::all<ComponentA>().get();
		Family &family2 = Family::all<ComponentB>().get();
		Family &family3 = Family::all<ComponentC>().get();
		Family &family4 = Family::all<ComponentA, ComponentB>().get();
		Family &family5 = Family::all<ComponentA, ComponentC>().get();
		Family &family6 = Family::all<ComponentB, ComponentA>().get();
		Family &family7 = Family::all<ComponentB, ComponentC>().get();
		Family &family8 = Family::all<ComponentC, ComponentA>().get();
		Family &family9 = Family::all<ComponentC, ComponentB>().get();
		Family &family10 = Family::all<ComponentA, ComponentB, ComponentC>().get();
		Family &family11 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();
		Family &family12 = Family::all<ComponentC, ComponentD>().one<ComponentE, ComponentF>()
			.exclude<ComponentA, ComponentB>().get();
		Family &family13 = Family::all().get();

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
		Family &family1 = Family::all<ComponentA>().one<ComponentB>().exclude<ComponentC>().get();
		Family &family2 = Family::all<ComponentB>().one<ComponentC>().exclude<ComponentA>().get();
		Family &family3 = Family::all<ComponentC>().one<ComponentA>().exclude<ComponentB>().get();
		Family &family4 = Family::all<ComponentA>().one<ComponentB>().exclude<ComponentC>().get();
		Family &family5 = Family::all<ComponentB>().one<ComponentC>().exclude<ComponentA>().get();
		Family &family6 = Family::all<ComponentC>().one<ComponentA>().exclude<ComponentB>().get();

		REQUIRE(family1 == family4);
		REQUIRE(family2 == family5);
		REQUIRE(family3 == family6);
		REQUIRE(family1 != family2);
		REQUIRE(family1 != family3);
	}

	TEST_CASE("entityMatch") {
		Family &family = Family::all<ComponentA, ComponentB>().get();

		Entity entity;
		ComponentA a;
		ComponentB b;
		entity.add(&a);
		entity.add(&b);

		REQUIRE(family.matches(&entity));

		entity.add(new ComponentC());

		REQUIRE(family.matches(&entity));
	}

	TEST_CASE("entityMismatch") {
		Family &family = Family::all<ComponentA, ComponentC>().get();

		Entity entity;
		ComponentA a;
		ComponentB b;
		entity.add(&a);
		entity.add(&b);

		REQUIRE(!family.matches(&entity));

		entity.remove<ComponentB>();

		REQUIRE(!family.matches(&entity));
	}

	TEST_CASE("entityMatchThenMismatch") {
		Family &family = Family::all<ComponentA, ComponentB>().get();

		Entity entity;
		ComponentA a;
		ComponentB b;
		entity.add(&a);
		entity.add(&b);

		REQUIRE(family.matches(&entity));

		entity.remove<ComponentA>();

		REQUIRE(!family.matches(&entity));
	}

	TEST_CASE("entityMismatchThenMatch") {
		Family &family = Family::all<ComponentA, ComponentB>().get();

		Entity entity;
		ComponentA a;
		ComponentB b;
		ComponentC c;
		entity.add(&a);
		entity.add(&c);

		REQUIRE(!family.matches(&entity));

		entity.add(&b);

		REQUIRE(family.matches(&entity));
	}

	TEST_CASE("testEmptyFamily") {
		Family &family = Family::all().get();
		Entity entity;
		REQUIRE(family.matches(&entity));
	}

	TEST_CASE("familyFiltering") {
		Family &family1 = Family::all<ComponentA, ComponentB>().one<ComponentC, ComponentD>()
			.exclude<ComponentE, ComponentF>().get();

		Family &family2 = Family::all<ComponentC, ComponentD>().one<ComponentA, ComponentB>()
			.exclude<ComponentE, ComponentF>().get();

		Entity entity;

		REQUIRE(!family1.matches(&entity));
		REQUIRE(!family2.matches(&entity));

		ComponentA a;
		ComponentB b;
		entity.add(&a);
		entity.add(&b);

		REQUIRE(!family1.matches(&entity));
		REQUIRE(!family2.matches(&entity));

		ComponentC c;
		entity.add(&c);

		REQUIRE(family1.matches(&entity));
		REQUIRE(!family2.matches(&entity));

		ComponentD d;
		entity.add(&d);

		REQUIRE(family1.matches(&entity));
		REQUIRE(family2.matches(&entity));

		ComponentE e;
		entity.add(&e);

		REQUIRE(!family1.matches(&entity));
		REQUIRE(!family2.matches(&entity));

		entity.remove<ComponentE>();

		REQUIRE(family1.matches(&entity));
		REQUIRE(family2.matches(&entity));

		entity.remove<ComponentA>();

		REQUIRE(!family1.matches(&entity));
		REQUIRE(family2.matches(&entity));
	}

	TEST_CASE("matchWithPooledEngine") {
		PooledEngine engine;
		TestSystemA systemA("A");
		TestSystemB systemB("B");
		engine.addSystem(&systemA);
		engine.addSystem(&systemB);

		Entity *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		Family &f = Family::all<ComponentB>().exclude<ComponentA>().get();

		REQUIRE(!f.matches(e));

		engine.clearPools();
	}

	TEST_CASE("matchWithPooledEngineInverse") {
		PooledEngine engine;

		engine.addSystem(new TestSystemA("A"));
		engine.addSystem(new TestSystemB("B"));

		Entity *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		Family &f = Family::all<ComponentA>().exclude<ComponentB>().get();

		REQUIRE(!f.matches(e));
		engine.clearPools();
	}

	TEST_CASE("matchWithoutSystems") {
		PooledEngine engine;

		Entity *e = engine.createEntity();
		e->add(engine.createComponent<ComponentB>());
		e->add(engine.createComponent<ComponentA>());
		engine.addEntity(e);

		Family &f = Family::all<ComponentB>().exclude<ComponentA>().get();

		REQUIRE(!f.matches(e));
		engine.clearPools();
	}

	TEST_CASE("matchWithComplexBuilding") {
		Family &family = Family::all<ComponentB>().one<ComponentA>().exclude<ComponentC>().get();
		Entity entity;
		entity.add(new ComponentA());
		REQUIRE(!family.matches(&entity));
		entity.add(new ComponentB());
		REQUIRE(family.matches(&entity));
		entity.add(new ComponentC());
		REQUIRE(!family.matches(&entity));
	}
}
