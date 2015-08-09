#include "benchpress.hpp"
#include "entityx/Entity.h"
#include "entityx/System.h"
#include <random>

namespace entityx_benchmarks {
	const int COMPONENT_VALUES = 8;
	const int NUM_ENTITIES = 2 << 14;
	
	using namespace entityx;
	struct ComponentMock1 { int values[COMPONENT_VALUES]; };
	struct ComponentMock2 { float values[COMPONENT_VALUES]; };
	struct ComponentMock3 { int values[COMPONENT_VALUES]; };
	struct ComponentMock4 { float values[COMPONENT_VALUES]; };
	struct ComponentMock5 { int values[COMPONENT_VALUES]; };
	struct ComponentMock6 { float values[COMPONENT_VALUES]; };
	struct ComponentMock7 { int values[COMPONENT_VALUES]; };
	struct ComponentMock8 { float values[COMPONENT_VALUES]; };
	
	template<typename T, int I>
	class EntitySystemMock : public System<EntitySystemMock<T, I>> {
	public:
		float inc = I / 100.0f;
		void update(entityx::EntityManager &entityManager, entityx::EventManager &eventManager, entityx::TimeDelta dt) override {
			ComponentHandle<T> c;
			for (auto entity : entityManager.entities_with_components<T>(c))
			{
				for(int i=0; i<COMPONENT_VALUES; i++) {
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
			m_systems.add<EntitySystemMock<ComponentMock3, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock3, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock4, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock4, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock5, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock5, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock6, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock6, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock7, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock7, -123>>();
			m_systems.add<EntitySystemMock<ComponentMock8, 123>>();
			m_systems.add<EntitySystemMock<ComponentMock8, -123>>();
			m_systems.configure();
			
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(0,8);
			for(int i=0; i<NUM_ENTITIES; i++) {
				auto e = m_entities.create();
				if(distribution(generator) == 1) e.assign<ComponentMock1>();
				if(distribution(generator) == 2) e.assign<ComponentMock2>();
				if(distribution(generator) == 3) e.assign<ComponentMock3>();
				if(distribution(generator) == 4) e.assign<ComponentMock4>();
				if(distribution(generator) == 5) e.assign<ComponentMock5>();
				if(distribution(generator) == 6) e.assign<ComponentMock6>();
				if(distribution(generator) == 7) e.assign<ComponentMock7>();
				if(distribution(generator) == 8) e.assign<ComponentMock8>();
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
