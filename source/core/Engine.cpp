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
#include <ecstasy/core/Engine.h>
#include <ecstasy/core/EntitySystem.h>

namespace ECS {
	bool compareSystems(EntitySystemBase *a, EntitySystemBase *b) {
		return a->priority < b->priority;
	}
	Engine:: Engine(int entityPoolInitialSize, int entityPoolMaxSize, int componentPoolInitialSize, int componentPoolMaxSize)
		: componentOperationHandler(*this), entityPool(entityPoolInitialSize, entityPoolMaxSize),
		componentPoolInitialSize(componentPoolInitialSize), componentPoolMaxSize(componentPoolMaxSize) {
		componentAdded.connect(this, &Engine::onComponentChange);
		componentRemoved.connect(this, &Engine::onComponentChange);
	}

	void Engine::onComponentChange(Entity *entity, ComponentBase *component) {
		if(!entity->scheduledForRemoval && entity->isValid())
			updateFamilyMembership(entity);
	}

	void Engine::addEntity(Entity *entity){
		if (entity->uuid != 0) throw std::invalid_argument("Entity already added to an engine");
		entity->uuid = obtainEntityId();
		if (updating || notifying) {
			auto *operation = entityOperationPool.obtain();
			operation->entity = entity;
			operation->type = EntityOperation::Type::Add;
			entityOperations.push_back(operation);
		}
		else {
			addEntityInternal(entity);
		}
	}

	void Engine::removeEntity(Entity *entity){
		if (updating || notifying) {
			if(entity->scheduledForRemoval)
				return;
			
			entity->scheduledForRemoval = true;
			auto *operation = entityOperationPool.obtain();
			operation->entity = entity;
			operation->type = EntityOperation::Type::Remove;
			entityOperations.push_back(operation);
		}
		else {
			entity->scheduledForRemoval = true;
			removeEntityInternal(entity);
		}
	}

	void Engine::removeAllEntities() {
		if (updating || notifying) {
			for(auto *entity: entities)
				entity->scheduledForRemoval = true;
			
			auto *operation = entityOperationPool.obtain();
			operation->type = EntityOperation::Type::RemoveAll;
			entityOperations.push_back(operation);
		}
		else {
			while(!entities.empty()) {
				removeEntity(entities.front());
			}
		}
	}

	Entity *Engine::getEntity(uint64_t id) const {
		auto it = entitiesById.find(id);
		if(it == entitiesById.end())
			return nullptr;
		return it->second;
	}

	void Engine::addSystem(EntitySystemBase *system){
		auto systemType = system->type;

		if (systemType >= systemsByType.size())
			systemsByType.resize(systemType + 1);
		else {
			auto *oldSystem = systemsByType[systemType];
			if(oldSystem)
				removeSystem(oldSystem);
		}
		systems.push_back(system);
		systemsByType[systemType] = system;
		system->addedToEngine(this);

		std::sort(systems.begin(), systems.end(), compareSystems);
	}

	void Engine::removeSystem(EntitySystemBase *system){
		if (system->type < systemsByType.size()) {
			auto *oldSystem = systemsByType[system->type];
			if(oldSystem) {
				systemsByType[system->type] = nullptr;

				auto it = std::find(systems.begin(), systems.end(), system);
				if(it != systems.end())
					systems.erase(it);
				system->removedFromEngine(this);
			}
		}
	}

	EntitySignal &Engine::getEntityAddedSignal(const Family &family) {
		registerFamily(family);
		return entityAddedSignals[&family];
	}

	EntitySignal &Engine::getEntityRemovedSignal(const Family &family) {
		registerFamily(family);
		return entityRemovedSignals[&family];
	}

	void Engine::update(float deltaTime){
		updating = true;
		for(auto *system: systems){
			if (system->checkProcessing())
				system->update(deltaTime);

			processComponentOperations();
			processPendingEntityOperations();
		}

		updating = false;
	}

	void Engine::updateFamilyMembership(Entity *entity){
		for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
			auto *family = it->first;
			auto &familyEntities = it->second;
			
			auto familyIndex = family->index;

			bool belongsToFamily = entity->getFamilyBits().get(familyIndex);
			bool matches = family->matches(entity);

			if (!belongsToFamily && matches) {
				familyEntities.push_back(entity);
				entity->familyBits.set(familyIndex);

				notifyFamilyListenersAdd(*family, entity);
			}
			else if (belongsToFamily && !matches) {
				auto it = std::find(familyEntities.begin(), familyEntities.end(), entity);
				if(it != familyEntities.end())
					familyEntities.erase(it);
				entity->familyBits.clear(familyIndex);

				notifyFamilyListenersRemove(*family, entity);
			}
		}
	}

	void Engine::removeEntityInternal(Entity *entity) {
		// Check if entity is able to be removed (id == 0 means either entity is not used by engine, or already removed/in pool)
		if (entity->getId() == 0) {
			if (entity->engine == this)
				entityPool.free(entity);
			return;
		}
		
		auto it = std::find(entities.begin(), entities.end(), entity);
		if(it == entities.end())
			throw std::invalid_argument("Entity does not belong to this engine");
		entities.erase(it);
		entitiesById.erase(entitiesById.find(entity->getId()));

		if(!entity->getFamilyBits().isEmpty()){
			for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
				auto *family = it->first;
				auto &familyEntities = it->second;

				if(family->matches(entity)){
					auto it2 = std::find(familyEntities.begin(), familyEntities.end(), entity);
					if(it2 != familyEntities.end())
						familyEntities.erase(it2);

					entity->familyBits.clear(family->index);
					notifyFamilyListenersRemove(*family, entity);
				}
			}
		}

		entity->componentOperationHandler = nullptr;

		notifying = true;
		entityRemoved.emit(entity);
		notifying = false;

		entityPool.free(entity);
	}

	void Engine::addEntityInternal(Entity *entity) {
		entities.push_back(entity);
		entitiesById.emplace(entity->getId(), entity);

		updateFamilyMembership(entity);

		entity->componentOperationHandler = &componentOperationHandler;

		notifying = true;
		entityAdded.emit(entity);
		notifying = false;
	}

	void Engine::notifyFamilyListenersAdd(const Family &family, Entity *entity) {
		auto it = entityAddedSignals.find(&family);
		if (it != entityAddedSignals.end()) {
			auto &signal = it->second;
			notifying = true;
			signal.emit(entity);
			notifying = false;
		}
	}

	void Engine::notifyFamilyListenersRemove(const Family &family, Entity *entity) {
		auto it = entityRemovedSignals.find(&family);
		if (it != entityRemovedSignals.end()) {
			auto &signal = it->second;
			notifying = true;
			signal.emit(entity);
			notifying = false;
		}
	}

	const std::vector<Entity *> *Engine::registerFamily(const Family &family) {
		auto it = entitiesByFamily.find(&family);
		if (it != entitiesByFamily.end())
			return &it->second;

		auto &familyEntities = entitiesByFamily[&family];
		for(auto *e : entities){
			if(family.matches(e)) {
				familyEntities.push_back(e);
				e->familyBits.set(family.index);
			}
		}
		return &familyEntities;
	}

	void Engine::processPendingEntityOperations() {
		for(auto *operation: entityOperations) {
			switch(operation->type) {
			case EntityOperation::Type::Add: addEntityInternal(operation->entity); break;
			case EntityOperation::Type::Remove: removeEntityInternal(operation->entity); break;
			case EntityOperation::Type::RemoveAll:
				while(!entities.empty()) {
					removeEntityInternal(entities.back());
				}
				break;
			default:
				throw std::runtime_error("Unexpected EntityOperation type");
			}

			entityOperationPool.free(operation);
		}

		entityOperations.clear();
	}

	void Engine::processComponentOperations() {
		for(auto *operation : componentOperations) {
			switch(operation->type) {
			case ComponentOperation::Type::Add: operation->entity->addInternal(operation->component); break;
			case ComponentOperation::Type::Remove: operation->entity->removeInternal(operation->componentType); break;
			case ComponentOperation::Type::RemoveAll: operation->entity->removeAllInternal(); break;
			default: break;
			}

			componentOperationsPool.free(operation);
		}

		componentOperations.clear();
	}

	Entity *Engine::createEntity() {
		auto *entity = entityPool.obtain();
		entity->engine = this;
		return entity;
	}

	void Engine::free(ComponentBase *component) {
		auto pool = componentPoolsByType[component->type];
		if (pool)
			pool->freeComponent(component);
	}

	void Engine::clearPools() {
		entityPool.clear();
		for (auto pool : componentPoolsByType){
			if (pool)
				delete pool;
		}
		componentPoolsByType.clear();
	}
}