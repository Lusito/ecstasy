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
#include <ecstasy/core/Engine.h>
#include <ecstasy/core/EntitySystem.h>

namespace ECS {
	bool compareSystems(EntitySystemBase *a, EntitySystemBase *b) {
		return a->priority < b->priority;
	}
	Engine:: Engine()
		: componentOperationHandler(*this) {
		componentAdded.connect(this, &Engine::onComponentChange);
		componentRemoved.connect(this, &Engine::onComponentChange);
	}

	void Engine::onComponentChange(Entity *entity, ComponentBase *component) {
		updateFamilyMembership(entity);
	}

	void Engine::addEntity(Entity *entity){
		entity->uuid = obtainEntityId();
		entity->engine = this;
		if (updating || notifying) {
			EntityOperation *operation = entityOperationPool.obtain();
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
			if(entity->scheduledForRemoval) {
				return;
			}
			entity->scheduledForRemoval = true;
			EntityOperation *operation = entityOperationPool.obtain();
			operation->entity = entity;
			operation->type = EntityOperation::Type::Remove;
			entityOperations.push_back(operation);
		}
		else {
			removeEntityInternal(entity);
		}
	}

	void Engine::removeAllEntities() {
		if (updating || notifying) {
			for(Entity *entity: entities) {
				entity->scheduledForRemoval = true;
			}
			EntityOperation *operation = entityOperationPool.obtain();
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
		SystemType systemType = system->type;

		if (!systemsByType.count(systemType)) {
			systems.push_back(system);
			systemsByType[systemType] = system;
			system->addedToEngine(this);

			std::sort(systems.begin(), systems.end(), compareSystems);
		}
	}

	void Engine::removeSystem(EntitySystemBase *system){
		auto it = std::find(systems.begin(), systems.end(), system);
		if(it != systems.end()) {
			systems.erase(it);
			auto it2 = systemsByType.find(system->type);
			if(it2 != systemsByType.end())
				systemsByType.erase(it2);
			system->removedFromEngine(this);
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
		for(EntitySystemBase *system: systems){
			if (system->checkProcessing())
				system->update(deltaTime);

			processComponentOperations();
			processPendingEntityOperations();
		}

		updating = false;
	}

	void Engine::updateFamilyMembership(Entity *entity){
		for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
			const Family *family = it->first;
			std::vector<Entity *> &familyEntities = it->second;
			
			int familyIndex = family->index;


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
		entity->scheduledForRemoval = false;
		auto itEnt = std::find(entities.begin(), entities.end(), entity);
		if(itEnt != entities.end())
			entities.erase(itEnt);
		auto itEntMap = entitiesById.find(entity->getId());
		if(itEntMap != entitiesById.end())
			entitiesById.erase(itEntMap);

		if(!entity->getFamilyBits().isEmpty()){
			for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
				const Family *family = it->first;
				std::vector<Entity *> &familyEntities = it->second;

				if(family->matches(entity)){
					auto itEnt2 = std::find(familyEntities.begin(), familyEntities.end(), entity);
					if(itEnt2 != familyEntities.end())
						familyEntities.erase(itEnt2);

					entity->familyBits.clear(family->index);
					notifyFamilyListenersRemove(*family, entity);
				}
			}
		}

		entity->componentOperationHandler = nullptr;

		notifying = true;
		entityRemoved.emit(entity);
		notifying = false;
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

		std::vector<Entity *> &familyEntities = entitiesByFamily[&family];
		for(Entity *e : entities){
			if(family.matches(e)) {
				familyEntities.push_back(e);
				e->familyBits.set(family.index);
			}
		}
		return &familyEntities;
	}

	void Engine::processPendingEntityOperations() {
		for(EntityOperation *operation: entityOperations) {
			
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
		for(ComponentOperation *operation : componentOperations) {
			switch(operation->type) {
			case ComponentOperation::Type::Add: operation->entity->addInternal(operation->component); break;
			case ComponentOperation::Type::Remove: operation->entity->removeInternal(operation->componentType); break;
			default: break;
			}

			componentOperationsPool.free(operation);
		}

		componentOperations.clear();
	}
}