#include "benchpress.hpp"
#include<ecstasy/core/Engine.h>
#include<ecstasy/systems/IteratingSystem.h>
#include <random>

namespace ecstasy_benchmarks {
	const int COMPONENT_VALUES = 8;
	const int NUM_ENTITIES = 2 << 14;
	
	struct ComponentMock1 : public Component<ComponentMock1> { int values[COMPONENT_VALUES]; };
	struct ComponentMock2 : public Component<ComponentMock2> { float values[COMPONENT_VALUES]; };
	struct ComponentMock3 : public Component<ComponentMock3> { int values[COMPONENT_VALUES]; };
	struct ComponentMock4 : public Component<ComponentMock4> { float values[COMPONENT_VALUES]; };
	struct ComponentMock5 : public Component<ComponentMock5> { int values[COMPONENT_VALUES]; };
	struct ComponentMock6 : public Component<ComponentMock6> { float values[COMPONENT_VALUES]; };
	struct ComponentMock7 : public Component<ComponentMock7> { int values[COMPONENT_VALUES]; };
	struct ComponentMock8 : public Component<ComponentMock8> { float values[COMPONENT_VALUES]; };
	
	template<typename T, int I>
	class EntitySystemMock : public IteratingSystem<EntitySystemMock<T, I>> {
	public:
		float inc = I / 100.0f;
		EntitySystemMock() : IteratingSystem<EntitySystemMock<T, I>>(Family::all<T>().get()) {}
		
		void processEntity(Entity *entity, float deltaTime) override {
			auto *c = entity->get<T>();
			for(int i=0; i<COMPONENT_VALUES; i++) {
				c->values[i] += inc;
			}
		}
	};
	
	class Benchmark  {
	public:
		Engine engine;
		Benchmark() {
			engine.addSystem<EntitySystemMock<ComponentMock1, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock1, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock2, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock2, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock3, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock3, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock4, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock4, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock5, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock5, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock6, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock6, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock7, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock7, -123>>();
			engine.addSystem<EntitySystemMock<ComponentMock8, 123>>();
			engine.addSystem<EntitySystemMock<ComponentMock8, -123>>();
			
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(0,8);
			for(int i=0; i<NUM_ENTITIES; i++) {
				auto *e = engine.createEntity();
				if(distribution(generator) == 1) e->assign<ComponentMock1>();
				if(distribution(generator) == 2) e->assign<ComponentMock2>();
				if(distribution(generator) == 3) e->assign<ComponentMock3>();
				if(distribution(generator) == 4) e->assign<ComponentMock4>();
				if(distribution(generator) == 5) e->assign<ComponentMock5>();
				if(distribution(generator) == 6) e->assign<ComponentMock6>();
				if(distribution(generator) == 7) e->assign<ComponentMock7>();
				if(distribution(generator) == 8) e->assign<ComponentMock8>();
				engine.addEntity(e);
			}
		}
		void run(benchpress::context *ctx) {
			for (size_t i = 0; i < ctx->num_iterations(); ++i) {
				engine.update(0);
			}
		}
	};
	BENCHMARK(Benchmark, "ECStasy");

}
