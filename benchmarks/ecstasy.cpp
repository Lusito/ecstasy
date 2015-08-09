#include "benchpress.hpp"
#include <ecstasy/core/Engine.h>
#include <ecstasy/systems/IteratingSystem.h>
#include <random>

namespace ecstasy_benchmarks {
	const int NUM_ENTITIES = 1 << 15;

	struct ComponentA : public Component<ComponentA> { float a; float b; float c; };
	struct ComponentB : public Component<ComponentB> { float a; float b; float c; };
	struct ComponentC : public Component<ComponentC> { float a; float b; float c; };
	struct ComponentD : public Component<ComponentD> { float a; float b; float c; };
	struct ComponentE : public Component<ComponentE> { float a; float b; float c; };

	struct IteratingSystemA : public IteratingSystem<IteratingSystemA> {
	public:
		IteratingSystemA(): IteratingSystem<IteratingSystemA>(Family::all<ComponentA>().get()) {
		}

		void processEntity(Entity *entity, float deltaTime) override {
			ComponentA *c = entity->get<ComponentA>();
			c->a++;
			c->b++;
			c->c++;
		}
	};

	struct IteratingSystemB : public IteratingSystem<IteratingSystemB> {
	public:
		IteratingSystemB(): IteratingSystem<IteratingSystemB>(Family::all<ComponentB>().get()) {
		}

		void processEntity(Entity *entity, float deltaTime) override {
			ComponentB *c = entity->get<ComponentB>();
			c->a++;
			c->b++;
			c->c++;
		}
	};

	struct IteratingSystemC : public IteratingSystem<IteratingSystemC> {
	public:
		IteratingSystemC(): IteratingSystem<IteratingSystemC>(Family::all<ComponentC>().get()) {
		}

		void processEntity(Entity *entity, float deltaTime) override {
			ComponentC *c = entity->get<ComponentC>();
			c->a++;
			c->b++;
			c->c++;
		}
	};

	struct IteratingSystemD : public IteratingSystem<IteratingSystemD> {
	public:
		IteratingSystemD(): IteratingSystem<IteratingSystemD>(Family::all<ComponentD>().get()) {
		}

		void processEntity(Entity *entity, float deltaTime) override {
			ComponentD *c = entity->get<ComponentD>();
			c->a++;
			c->b++;
			c->c++;
		}
	};

	struct IteratingSystemE : public IteratingSystem<IteratingSystemE> {
	public:
		IteratingSystemE(): IteratingSystem<IteratingSystemE>(Family::all<ComponentE>().get()) {
		}

		void processEntity(Entity *entity, float deltaTime) override {
			ComponentE *c = entity->get<ComponentE>();
			c->a++;
			c->b++;
			c->c++;
		}
	};

	class Benchmark  {
	public:
		Engine engine;

		Benchmark() {
			engine.addSystem<IteratingSystemA>();
			engine.addSystem<IteratingSystemB>();
			engine.addSystem<IteratingSystemC>();
			engine.addSystem<IteratingSystemD>();
			engine.addSystem<IteratingSystemE>();

			std::vector<int> v;

			for (int i = 0; i < NUM_ENTITIES; i++) {
				v.push_back(i);
			}

			std::mt19937 g(0);
			std::shuffle(v.begin(), v.end(), g);

			for (int i = 0; i < NUM_ENTITIES; i++) {
				Entity *entity = engine.createEntity();
				if (v[i] & 1)  entity->assign<ComponentA>();
				if (v[i] & 2)  entity->assign<ComponentB>();
				if (v[i] & 4)  entity->assign<ComponentC>();
				if (v[i] & 8)  entity->assign<ComponentD>();
				if (v[i] & 16) entity->assign<ComponentE>();
				engine.addEntity(entity);
			}
		}

		void run(benchpress::context *ctx) {
			for (size_t i = 0; i < ctx->num_iterations(); ++i) {
				engine.update(42.0);
			}
		}
	};
	BENCHMARK(Benchmark, "ECS-Tasy");

}
