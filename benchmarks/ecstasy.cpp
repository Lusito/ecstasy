#include "benchpress.hpp"
#include<ecstasy/core/Engine.h>
#include<ecstasy/systems/IteratingSystem.h>
#include <random>

namespace ecstasy_benchmarks {
	struct ComponentMock1 : public Component<ComponentMock1> {
		int values[256];
	};
	
	struct ComponentMock2 : public Component<ComponentMock2> {
		float values[256];
	};
	
	template<typename T, int I>
	class EntitySystemMock : public IteratingSystem<EntitySystemMock<T, I>> {
	public:
		float inc = I / 100.0f;
		EntitySystemMock() : IteratingSystem<EntitySystemMock<T, I>>(Family::all<T>().get()) {}
		
		void processEntity(Entity *entity, float deltaTime) override {
			auto *c = entity->get<T>();
			for(int i=0; i<256; i++) {
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
			
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(0,3);
			for(int i=0; i<10000; i++) {
				auto *e = engine.createEntity();
				switch(distribution(generator)) {
				case 1:
					e->assign<ComponentMock1>();
					break;
				case 2:
					e->assign<ComponentMock2>();
					break;
				case 3:
					e->assign<ComponentMock1>();
					e->assign<ComponentMock2>();
					break;
				}
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
