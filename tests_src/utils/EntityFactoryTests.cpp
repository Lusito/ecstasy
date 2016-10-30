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
#include <ecstasy/utils/ComponentFactory.hpp>
#include <ecstasy/utils/EntityFactory.hpp>

#define NS_TEST_CASE(name) TEST_CASE("EntityFactory: " name)
namespace EntityFactoryTests {

#define DECLARE_COMPONENT_FACTORY(ComponentFactoryName)\
	class ComponentFactoryName : public ComponentFactory {\
	public:\
		ComponentFactoryName() {}\
		ComponentFactoryName(const ComponentFactoryName& orig) = delete;\
		~ComponentFactoryName() {}\
		bool assemble(Entity *entity, ComponentBlueprint &blueprint) override;\
	};

	struct PositionComponent: public Component<PositionComponent> {
		float x = 0;
		float y = 0;
	};
	struct RenderComponent: public Component<RenderComponent> {
		int layer = 0;
		std::string color;
	};
	struct LabelComponent: public Component<LabelComponent> {
		std::string message;
	};
	struct MarkerComponent: public Component<MarkerComponent> {
		std::string message;
	};

	DECLARE_COMPONENT_FACTORY(PositionComponentFactory)
	bool PositionComponentFactory::assemble(Entity *entity, ComponentBlueprint &blueprint) {
		auto comp = entity->emplace<PositionComponent>();
		comp->x = blueprint.getFloat("x", 1);
		comp->y = blueprint.getFloat("y", 2);
		return true;
	}
	DECLARE_COMPONENT_FACTORY(RenderComponentFactory)
	bool RenderComponentFactory::assemble(Entity *entity, ComponentBlueprint &blueprint) {
		auto comp = entity->emplace<RenderComponent>();
		comp->layer = blueprint.getFloat("layer", 1);
		comp->color = blueprint.getString("color", "FFFFFF");
		return true;
	}
	DECLARE_COMPONENT_FACTORY(LabelComponentFactory)
	bool LabelComponentFactory::assemble(Entity *entity, ComponentBlueprint &blueprint) {
		auto comp = entity->emplace<LabelComponent>();
		comp->message = blueprint.getString("message", "no message");
		return true;
	}

	Entity* testFactoryInit(const std::string &filename, Engine& engine) {
		auto factory = std::make_shared<EntityFactory>();

		// Setup component factories
		factory->addComponentFactory<PositionComponentFactory>("Position");
		factory->addComponentFactory<RenderComponentFactory>("Render");
		factory->addComponentFactory<LabelComponentFactory>("Label");
		factory->addComponentFactory<SimpleComponentFactory<MarkerComponent>>("Marker");

		std::shared_ptr<EntityBlueprint> entityBlueprint;
		std::string error = parseBlueprint(filename, entityBlueprint);
		REQUIRE(error.empty());
		factory->addEntityBlueprint("good", entityBlueprint);

		engine.setEntityFactory(factory);
		auto entity = engine.assembleEntity("good");
		REQUIRE(entity != nullptr);
		REQUIRE(entity->has<PositionComponent>());
		REQUIRE(entity->has<RenderComponent>());
		REQUIRE(entity->has<LabelComponent>());
		REQUIRE(entity->has<MarkerComponent>());
		return entity;
	}

	NS_TEST_CASE("test_entity_factory_good") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		auto entity = testFactoryInit("tests_assets/good.def", engine);

		auto pos = entity->get<PositionComponent>();
		REQUIRE(pos->x == 10.1f);
		REQUIRE(pos->y == 11.2f);
		auto render = entity->get<RenderComponent>();
		REQUIRE(render->layer == 42);
		REQUIRE(render->color == "FF0000");
		auto label = entity->get<LabelComponent>();
		REQUIRE(label->message == "a full blown message");
		engine.addEntity(entity);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("test_entity_factory_good_defaults") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		auto entity = testFactoryInit("tests_assets/good_defaults.def", engine);

		auto pos = entity->get<PositionComponent>();
		REQUIRE(pos->x == 1);
		REQUIRE(pos->y == 2);
		auto render = entity->get<RenderComponent>();
		REQUIRE(render->layer == 1);
		REQUIRE(render->color == "FFFFFF");
		auto label = entity->get<LabelComponent>();
		REQUIRE(label->message == "no message");
		engine.addEntity(entity);
		TEST_MEMORY_LEAK_END
	}
}
