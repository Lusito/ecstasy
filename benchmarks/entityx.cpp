#include "benchpress.hpp"
#include "entityx/Entity.h"
#include "entityx/System.h"
#include <random>

namespace entityx_benchmarks {
	using namespace entityx;
	struct ComponentMock1 {
		int values[256];
	};
	
	struct ComponentMock2 {
		float values[256];
	};
	
	template<typename T, int I>
	class EntitySystemMock : public System<EntitySystemMock<T, I>> {
	public:
		float inc = I / 100.0f;
		void update(entityx::EntityManager &entityManager, entityx::EventManager &eventManager, entityx::TimeDelta dt) override {
			ComponentHandle<T> c;
			for (auto entity : entityManager.entities_with_components<T>(c))
			{
				for(int i=0; i<256; i++) {
					c->values[i] += inc;
				}
			}
		}
	};
	
	class Benchmark  {
	public:
		EventManager m_events;
		EntityManager m_entities;
		SystemManager m_systems;
		Benchmark() :  m_entities(m_events), m_systems(m_entities, m_events) {
			m_systems.add<EntitySystemMock<ComponentMock1, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock1, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock2, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock2, -123>>();
			
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(0,3);
			for(int i=0; i<10000; i++) {
				auto e = m_entities.create();
				switch(distribution(generator)) {
				case 1:
					e.assign<ComponentMock1>();
					break;
				case 2:
					e.assign<ComponentMock2>();
					break;
				case 3:
					e.assign<ComponentMock1>();
					e.assign<ComponentMock2>();
					break;
				}
			}
		}
		void run(benchpress::context *ctx) {
			for (size_t i = 0; i < ctx->num_iterations(); ++i) {
				m_systems.update_all(0);
			}
		}
	};
	BENCHMARK(Benchmark, "entityx");

}
