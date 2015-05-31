#pragma once
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

#include "Types.h"
#include "EntitySystem.h"
#include "Entity.h"
#include "ComponentOperations.h"
#include "EntityOperations.h"
#include "Family.h"
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <signal11/Signal.h>

namespace ECS {
	typedef Signal11::Signal<void(Entity *, ComponentBase *)> ComponentSignal;
	typedef Signal11::Signal<void(Entity *)> EntitySignal;
		
	/**
	 * The heart of the Entity framework. It is responsible for keeping track of {@link Entity} and
	 * managing {@link EntitySystem} objects. The Engine should be updated every tick via the {@link #update(float)} method.
	 *
	 * With the Engine you can:
	 *
	 * <ul>
	 * <li>Add/Remove {@link Entity} objects</li>
	 * <li>Add/Remove {@link EntitySystem}s</li>
	 * <li>Obtain a list of entities for a specific {@link Family}</li>
	 * <li>Update the main loop</li>
	 * <li>Register/unregister {@link EntityListener} objects</li>
	 * </ul>
	 *
	 * @author Stefan Bachmann
	 */
	class Engine {
	private:
		friend class ComponentOperationHandler;
		friend class Entity;

		std::vector<Entity *> entities;
		std::unordered_map<uint64_t, Entity *> entitiesById;

		std::vector<EntityOperation *> entityOperations;
		EntityOperationPool entityOperationPool;

		std::vector<EntitySystemBase *> systems;
		std::unordered_map<SystemType, EntitySystemBase *> systemsByType;

		std::unordered_map<const Family *, std::vector<Entity *>> entitiesByFamily;

		std::unordered_map<const Family *, EntitySignal> entityAddedSignals;
		std::unordered_map<const Family *, EntitySignal> entityRemovedSignals;

		bool updating = false;

		bool notifying = false;
		uint64_t nextEntityId = 1;

		/** Mechanism to delay component addition/removal to avoid affecting system processing */
		ComponentOperationPool componentOperationsPool;
		std::vector<ComponentOperation *> componentOperations;
		ComponentOperationHandler componentOperationHandler;

	public:
		/** Will dispatch an event when a component is added. */
		ComponentSignal componentAdded;
		/** Will dispatch an event when a component is removed. */
		ComponentSignal componentRemoved;
		/** Will dispatch an event when an entity is added. */
		EntitySignal entityAdded;
		/** Will dispatch an event when an entity is removed. */
		EntitySignal entityRemoved;
		
	public:
		Engine();
		void onComponentChange(Entity* entity, ComponentBase* component);

		virtual ~Engine() {
			// fixme: is this safe ?
			clear();
		}

		// fixme: if an engine gets deleted before remaining entities, etc. this is currently used in the testcases, not sure if its actually useful in real world code
		void clear() {
			processComponentOperations();
			processPendingEntityOperations();
			removeAllEntities();
		}

		uint64_t obtainEntityId() {
			return nextEntityId++;
		}

		/**
		 * Adds an entity to this Engine.
		 */
		void addEntity(Entity *entity);

		/**
		 * Removes an entity from this Engine.
		 */
		void removeEntity(Entity *entity);

		/**
		 * Removes all entities registered with this Engine.
		 */
		void removeAllEntities();

		Entity *getEntity(uint64_t id) const;

		const std::vector<Entity *> *getEntities() const {
			return &entities;
		}

		/**
		 * Adds the {@link EntitySystem} to this Engine.
		 */
		void addSystem(EntitySystemBase *system);

		/**
		 * Removes the {@link EntitySystem} from this Engine.
		 */
		void removeSystem(EntitySystemBase *system);

		/**
		 * Quick {@link EntitySystem} retrieval.
		 */
		template<typename T>
		T *getSystem() const {
			auto it = systemsByType.find(getSystemType<T>());
			if(it == systemsByType.end())
				return nullptr;
			return (T *) it->second;
		}

		/**
		 * @return immutable array of all entity systems managed by the {@link Engine}.
		 */
		const std::vector<EntitySystemBase *> &getSystems() const {
			return systems;
		}

		/**
		 * Returns immutable collection of entities for the specified {@link Family}. Will return the same instance every time.
		 */
		const std::vector<Entity *> *getEntitiesFor(const Family &family) {
			return registerFamily(family);
		}

		/**
		* Get the EntitySignal which emits when an entity is added to a family
		*/
		EntitySignal &getEntityAddedSignal(const Family &family);

		/**
		* Get the EntitySignal which emits when an entity is removed from a family
		*/
		EntitySignal &getEntityRemovedSignal(const Family &family);

		/**
		 * Updates all the systems in this Engine.
		 * @param deltaTime The time passed since the last frame.
		 */
		void update(float deltaTime);

	private:
		void updateFamilyMembership(Entity *entity);

	protected:
		virtual void removeEntityInternal(Entity *entity);

		virtual void addEntityInternal(Entity *entity);

	private:
		void notifyFamilyListenersAdd(const Family &family, Entity *entity);

		void notifyFamilyListenersRemove(const Family &family, Entity *entity);

		const std::vector<Entity *> *registerFamily(const Family &family);

		void processPendingEntityOperations();

		void processComponentOperations();
	};
}