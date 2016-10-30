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
#include <ecstasy/core/Engine.hpp>
#include <ecstasy/core/Family.hpp>
#include <ecstasy/core/EntitySystem.hpp>
#include <ecstasy/utils/EntityFactory.hpp>
#include <ecstasy/utils/DefaultMemoryManager.hpp>

namespace ecstasy {
	bool compareSystems(std::shared_ptr<EntitySystemBase>& a, std::shared_ptr<EntitySystemBase>& b) {
		return a->getPriority() < b->getPriority();
	}

	Engine::Engine()
		: Engine(std::make_shared<DefaultMemoryManager>()) {
	}

	Engine::Engine(std::shared_ptr<MemoryManager> memoryManager)
		: memoryManager(memoryManager), entityOperationHandler(*this, memoryManager.get()),
		componentOperationHandler(*this, memoryManager.get()) {
		componentAdded.connect(this, &Engine::onComponentChange);
		componentRemoved.connect(this, &Engine::onComponentChange);
	}

	Engine::~Engine(){
		do {
			removeAllEntities();
			componentOperationHandler.process();
			entityOperationHandler.process();
		} while (!entities.empty());

		removeAllSystems();
	}

	void Engine::onComponentChange(Entity* entity, ComponentBase* component) {
		if(!entity->scheduledForRemoval && entity->isValid())
			updateFamilyMembership(entity);
	}

	void Engine::addEntity(Entity* entity) {
		if (entity->uuid != 0) throw std::invalid_argument("Entity already added to an engine");
		entity->uuid = obtainEntityId();
		if (updating || notifying)
			entityOperationHandler.add(entity);
		else
			addEntityInternal(entity);
	}

	void Engine::removeEntity(Entity* entity) {
		if (updating || notifying) {
			if(entity->scheduledForRemoval)
				return;

			entity->scheduledForRemoval = true;
			entityOperationHandler.remove(entity);
		}
		else {
			entity->scheduledForRemoval = true;
			removeEntityInternal(entity);
		}
	}

	void Engine::removeAllEntities() {
		if (updating || notifying) {
			for(auto entity: entities)
				entity->scheduledForRemoval = true;

			entityOperationHandler.removeAll();
		}
		else {
			while(!entities.empty()) {
				removeEntity(entities.front());
			}
		}
	}

	Entity* Engine::getEntity(uint64_t id) const {
		auto it = entitiesById.find(id);
		if(it == entitiesById.end())
			return nullptr;
		return it->second;
	}

	void Engine::addSystem(std::shared_ptr<EntitySystemBase> system) {
		auto systemType = system->type;

		if (systemType >= systemsByType.size())
			systemsByType.resize(systemType + 1);
		else
			removeSystemInternal(systemType);

		systems.push_back(system);
		systemsByType[systemType] = system;
		system->engine = this;
		system->addedToEngine(this);

		sortSystems();
	}

	void Engine::removeSystemInternal(const SystemType& type) {
		if (type < systemsByType.size()) {
			auto system = systemsByType[type];
			if(system) {
				systemsByType[type].reset();

				auto it = std::find(systems.begin(), systems.end(), system);
				if(it != systems.end())
					systems.erase(it);
				system->removedFromEngine(this);
				system->engine = nullptr;
			}
		}
	}

	void Engine::removeAllSystems() {
		for (auto system : systems) {
			system->removedFromEngine(this);
			system->engine = nullptr;
		}
		systems.clear();
		systemsByType.clear();
	}

	void Engine::sortSystems() {
		std::sort(systems.begin(), systems.end(), compareSystems);
	}

	EntitySignal& Engine::getEntityAddedSignal(const Family& family) {
		registerFamily(family);
		return entityAddedSignals[&family];
	}

	EntitySignal& Engine::getEntityRemovedSignal(const Family& family) {
		registerFamily(family);
		return entityRemovedSignals[&family];
	}

	void Engine::update(float deltaTime){
		updating = true;
		for(auto system: systems){
			if (system->checkProcessing())
				system->update(deltaTime);

			componentOperationHandler.process();
			entityOperationHandler.process();
		}

		updating = false;
	}

	void Engine::updateFamilyMembership(Entity* entity){
		for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
			auto family = it->first;
			auto& familyEntities = it->second;

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

	void Engine::removeEntityInternal(Entity* entity) {
		// Check if entity is able to be removed (id == 0 means the entity has not been added to the engine yet)
		if (entity->getId() == 0) {
			if (entity->engine == this) {
				entity->~Entity();
				memoryManager->free(sizeof(Entity), alignof(Entity), entity);
			}
			return;
		}

		auto it = std::find(entities.begin(), entities.end(), entity);
		if(it == entities.end())
			throw std::invalid_argument("Entity does not belong to this engine");
		entities.erase(it);
		entitiesById.erase(entitiesById.find(entity->getId()));

		if(!entity->getFamilyBits().isEmpty()){
			for (auto it = entitiesByFamily.begin(); it != entitiesByFamily.end(); it++) {
				auto family = it->first;
				auto& familyEntities = it->second;

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

		entity->~Entity();
		memoryManager->free(sizeof(Entity), alignof(Entity), entity);
	}

	void Engine::addEntityInternal(Entity* entity) {
		entities.push_back(entity);
		entitiesById.emplace(entity->getId(), entity);

		updateFamilyMembership(entity);

		entity->componentOperationHandler = &componentOperationHandler;

		notifying = true;
		entityAdded.emit(entity);
		notifying = false;
	}

	void Engine::notifyFamilyListenersAdd(const Family& family, Entity* entity) {
		auto it = entityAddedSignals.find(&family);
		if (it != entityAddedSignals.end()) {
			auto& signal = it->second;
			notifying = true;
			signal.emit(entity);
			notifying = false;
		}
	}

	void Engine::notifyFamilyListenersRemove(const Family& family, Entity* entity) {
		auto it = entityRemovedSignals.find(&family);
		if (it != entityRemovedSignals.end()) {
			auto& signal = it->second;
			notifying = true;
			signal.emit(entity);
			notifying = false;
		}
	}

	const std::vector<Entity*>* Engine::registerFamily(const Family& family) {
		auto it = entitiesByFamily.find(&family);
		if (it != entitiesByFamily.end())
			return &it->second;

		auto& familyEntities = entitiesByFamily[&family];
		for(auto e : entities){
			if(family.matches(e)) {
				familyEntities.push_back(e);
				e->familyBits.set(family.index);
			}
		}
		return& familyEntities;
	}

	Entity* Engine::createEntity() {
		auto memory = memoryManager->allocate(sizeof(Entity), alignof(Entity));
		auto entity = new(memory)Entity();
		entity->engine = this;
		entity->memoryManager = memoryManager.get();
		return entity;
	}

	Entity* Engine::assembleEntity(const std::string& blueprintname) {
		if(!entityFactory)
			return nullptr;

		auto entity = createEntity();
		if(!entityFactory->assemble(entity, blueprintname)) {
			entity->~Entity();
			memoryManager->free(sizeof(Entity), alignof(Entity), entity);
			entity = nullptr;
		}
		return entity;
	}

	void Engine::reduceMemory() {
		memoryManager->reduceMemory();
	}
}