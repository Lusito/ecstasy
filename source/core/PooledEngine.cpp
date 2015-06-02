#include <ecstasy/core/PooledEngine.h>

namespace ECS {
	ComponentBase *PooledEntity::removeInternal(ComponentType type) {
		auto *component = Entity::removeInternal(type);

		if (component != nullptr) {
			auto pool = engine->componentPoolsByType[type];
			pool->freeComponent(component);
		}

		return component;
	}

	void PooledEntity::reset() {
		removeAll();
		uuid = 0;
		flags = 0;
		scheduledForRemoval = false;
		engine = nullptr;
	}

	Entity *PooledEngine::createEntity() {
		auto *entity = entityPool.obtain();
		entity->engine = this;
		return entity;
	}

	void PooledEngine::free(ComponentBase *component) {
		auto pool = componentPoolsByType[component->type];
		if (pool)
			pool->freeComponent(component);
	}

	void PooledEngine::clearPools() {
		entityPool.clear();
		for (auto pool : componentPoolsByType){
			if (pool)
				delete pool;
		}
		componentPoolsByType.clear();
	}

	void PooledEngine::removeEntityInternal(Entity *entity) {
		// Check if entity is able to be removed (id == 0 means either entity is not used by engine, or already removed/in pool)
		if (entity->getId() == 0) return;

		Engine::removeEntityInternal(entity);

		entityPool.free((PooledEntity *)entity);
	}
}