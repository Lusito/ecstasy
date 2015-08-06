#pragma once
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

#include "Types.h"
#include "EntitySystem.h"
#include "Entity.h"
#include "Component.h"
#include "ComponentAllocator.h"
#include "EntityOperations.h"
#include "Family.h"
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <signal11/Signal.h>

namespace ECS {
	/// Signal11::Signal for Component signals
	typedef Signal11::Signal<void(Entity *, ComponentBase *)> ComponentSignal;
	/// Signal11::Signal for Entity signals
	typedef Signal11::Signal<void(Entity *)> EntitySignal;

	/**
	 * The heart of the Entity framework. It is responsible for keeping track of Entity and
	 * managing EntitySystem objects. The Engine should be updated every tick via the update(float) method.
	 * Supports Entity pooling. This improves performance in environments where creating/deleting
	 * entities is frequent as it greatly reduces memory allocation.
	 *
	 * With the Engine you can:
	 *
	 * <ul>
	 * <li>Create entities using createEntity()</li>
	 * <li>Create components using createComponent()</li>
	 * <li>Add/Remove Entity objects</li>
	 * <li>Add/Remove {@link EntitySystem}s</li>
	 * <li>Obtain a list of entities for a specific Family</li>
	 * <li>Update the main loop</li>
	 * <li>Connect to/Disconnect from EntitySignal</li>
	 * </ul>
	 */
	class Engine : public ComponentAllocator {
	private:
		friend class ComponentOperationHandler;
		friend class EntityOperationHandler;
		friend class Entity;

		std::vector<Entity *> entities;
		std::unordered_map<uint64_t, Entity *> entitiesById;

		std::vector<EntitySystemBase *> systems;
		std::vector<EntitySystemBase *> systemsByType;

		std::unordered_map<const Family *, std::vector<Entity *>> entitiesByFamily;

		std::unordered_map<const Family *, EntitySignal> entityAddedSignals;
		std::unordered_map<const Family *, EntitySignal> entityRemovedSignals;

		bool updating = false;

		bool notifying = false;
		uint64_t nextEntityId = 1;

		// Mechanism to delay component addition/removal to avoid affecting system processing
		EntityOperationHandler entityOperationHandler;
		ComponentOperationHandler componentOperationHandler;

		Pool<Entity> entityPool;

	public:
		/// Will dispatch an event when a component is added.
		ComponentSignal componentAdded;
		/// Will dispatch an event when a component is removed.
		ComponentSignal componentRemoved;
		/// Will dispatch an event when an entity is added.
		EntitySignal entityAdded;
		/// Will dispatch an event when an entity is removed.
		EntitySignal entityRemoved;
		
	public:
		/**
		 * Creates a new Engine with the specified pool size configuration.
		 * 
		 * @param entityPoolInitialSize initial number of pre-allocated entities.
		 * @param entityPoolMaxSize maximum number of pooled entities.
		 */
		Engine (int entityPoolInitialSize = 10, int entityPoolMaxSize = 100);

		virtual ~Engine() {
			do {
				removeAllEntities();
				componentOperationHandler.process();
				entityOperationHandler.process();
			} while (!entities.empty());

			removeAllSystems();
		}
		
		/// @return Clean Entity from the Engine pool. In order to add it to the Engine, use addEntity(Entity).
		Entity *createEntity();

		/**
		 * Removes all free entities and operations from the pools to free up memory.
		 */
		void clearPools();

		/**
		 * Adds an entity to this Engine.
		 * 
		 * @param entity the entity to add
		 */
		void addEntity(Entity *entity);

		/**
		 * Removes an entity from this Engine.
		 * 
		 * @param entity the entity to remove
		 */
		void removeEntity(Entity *entity);

		/**
		 * Removes all entities registered with this Engine.
		 */
		void removeAllEntities();

		/**
         * @param id The id of an Entity
         * @return The entity associated with the specified id or null if no such entity exists.
         */
		Entity *getEntity(uint64_t id) const;

		/// @return a list of all entities
		const std::vector<Entity *> *getEntities() const {
			return &entities;
		}


		/**
		 * Adds a new system
		 *
		 * @tparam T The System class
		 * @param ... The constructor arguments
		 */
		template <typename T, typename ... Args>
		T *addSystem(Args && ... args) {
			auto system = new T(std::forward<Args>(args) ...);
			addSystemInternal(system);
			return system;
		}

		/**
		 * Removes the EntitySystem from this Engine.
		 *
		 * @tparam T The System class to remove
		 */
		template <typename T, typename ... Args>
		void removeSystem() {
			removeSystemInternal(getSystemType<T>());
		}

		/**
		 * Removes all systems registered with this Engine.
		 */
		void removeAllSystems();
		
		/**
		 * Sort all systems (usually done automatically, except if you override EntitySystem::getPriority())
		 */
		void sortSystems();

	private:
		/**
		 * Adds the EntitySystem to this Engine.
		 * 
		 * @param system The EntitySystem to add
		 */
		void addSystemInternal(EntitySystemBase *system);

		/**
		 * Removes the EntitySystem from this Engine.
		 * 
		 * @param type The EntitySystem type to remove
		 */
		void removeSystemInternal(const SystemType &type);

	public:
		/**
		 * Quick EntitySystem retrieval.
		 * 
		 * @tparam T The EntitySystem class
		 * @return The EntitySystem of the specified class, or null if no such system exists.
		 */
		template<typename T>
		T *getSystem() const {
			auto type = getSystemType<T>();
			if (type >= systemsByType.size())
				return nullptr;
			return (T *)systemsByType[type];
		}

		/**
		 * @return A list of all entity systems managed by the Engine.
		 */
		const std::vector<EntitySystemBase *> &getSystems() const {
			return systems;
		}

		/**
		 * @param family A Family instance
		 * @return A list of entities for the specified Family. Will return the same instance every time.
		 */
		const std::vector<Entity *> *getEntitiesFor(const Family &family) {
			return registerFamily(family);
		}

		/**
		 * @param family A Family instance
		 * @return The EntitySignal which emits when an entity is added to the specified Family
		 */
		EntitySignal &getEntityAddedSignal(const Family &family);

		/**
		 * @param family A Family instance
		 * @return The EntitySignal which emits when an entity is removed from the specified Family
		 */
		EntitySignal &getEntityRemovedSignal(const Family &family);

		/**
		 * Updates all the systems in this Engine.
		 * 
		 * @param deltaTime The time passed since the last frame.
		 */
		void update(float deltaTime);

	private:		
		void onComponentChange(Entity* entity, ComponentBase* component);

		uint64_t obtainEntityId() {
			return nextEntityId++;
		}
		
		void updateFamilyMembership(Entity *entity);

		void removeEntityInternal(Entity *entity);

		void addEntityInternal(Entity *entity);

		void notifyFamilyListenersAdd(const Family &family, Entity *entity);

		void notifyFamilyListenersRemove(const Family &family, Entity *entity);

		const std::vector<Entity *> *registerFamily(const Family &family);
	};
}

#ifdef USING_ECSTASY
	using ECS::Engine;
#endif
