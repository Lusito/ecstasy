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

	
/// \cond HIDDEN_SYMBOLS
	class ComponentPoolBase {
	public:
		virtual ~ComponentPoolBase() {}
		virtual void freeComponent(ComponentBase *object) = 0;
	};

	template<typename T>
	class ComponentPool : public ComponentPoolBase, public ReflectionPool<T> {
	public:
		ComponentPool(int initialSize, int maxSize) : ReflectionPool<T>(initialSize, maxSize) {}

		void freeComponent(ComponentBase *object) override {
			ReflectionPool<T>::free((T*)object);
		}
	};

	/** Entity Pool */
	class EntityPool : public Pool<Entity> {
	public:
		EntityPool(int initialSize, int maxSize) : Pool<Entity>(initialSize, maxSize) {}

	protected:
		Entity *newObject() override {
			return new Entity();
		}
	};
/// \endcond

	/**
	 * The heart of the Entity framework. It is responsible for keeping track of {@link Entity} and
	 * managing {@link EntitySystem} objects. The Engine should be updated every tick via the {@link update(float)} method.
	 * Supports {@link Entity} and {@link Component} pooling. This improves performance in environments where creating/deleting
	 * entities is frequent as it greatly reduces memory allocation.
	 *
	 * With the Engine you can:
	 *
	 * <ul>
	 * <li>Create entities using {@link createEntity()}</li>
	 * <li>Create components using {@link createComponent()}</li>
	 * <li>Add/Remove {@link Entity} objects</li>
	 * <li>Add/Remove {@link EntitySystem}s</li>
	 * <li>Obtain a list of entities for a specific {@link Family}</li>
	 * <li>Update the main loop</li>
	 * <li>Register/unregister {@link EntityListener} objects</li>
	 * </ul>
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

		// Mechanism to delay component addition/removal to avoid affecting system processing
		ComponentOperationPool componentOperationsPool;
		std::vector<ComponentOperation *> componentOperations;
		ComponentOperationHandler componentOperationHandler;

		int componentPoolInitialSize, componentPoolMaxSize;
		std::vector<ComponentPoolBase *> componentPoolsByType;
		EntityPool entityPool;

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
		 * Creates new Engine with the specified pools size configurations.
		 * 
		 * @param entityPoolInitialSize initial number of pre-allocated entities.
		 * @param entityPoolMaxSize maximum number of pooled entities.
		 * @param componentPoolInitialSize initial size for each component type pool.
		 * @param componentPoolMaxSize maximum size for each component type pool.
		 */
		Engine (int entityPoolInitialSize = 10, int entityPoolMaxSize = 100, int componentPoolInitialSize = 10, int componentPoolMaxSize = 100);

		virtual ~Engine() {
			processComponentOperations();
			processPendingEntityOperations();
			removeAllEntities();
			clearPools();
		}
		
		/// @return Clean {@link Entity} from the Engine pool. In order to add it to the {@link Engine}, use {@link addEntity(Entity)}.
		Entity *createEntity();

		/**
		 * Retrieves a new {@link Component} from the {@link Engine} pool. It will be placed back in the pool whenever it's removed
		 * from an {@link Entity} or the {@link Entity} itself is removed.
		 * 
		 * @tparam T The Component class
		 */
		template<typename T>
		T *createComponent() {
			return getOrCreateComponentPool<T>()->obtain();
		}
		
		void free(ComponentBase *component);

		/**
		 * Removes all free entities and components from their pools to free up memory.
		 */
		void clearPools();
		
		void onComponentChange(Entity* entity, ComponentBase* component);

		uint64_t obtainEntityId() {
			return nextEntityId++;
		}

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
		 * 
		 * @tparam T The EntitySystem class
		 */
		template<typename T>
		T *getSystem() const {
			auto it = systemsByType.find(getSystemType<T>());
			if(it == systemsByType.end())
				return nullptr;
			return (T *) it->second;
		}

		/**
		 * @return A list of all entity systems managed by the {@link Engine}.
		 */
		const std::vector<EntitySystemBase *> &getSystems() const {
			return systems;
		}

		/**
		 * @return The EntitySignal which emits when an entity is added to the specified Family
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
		void updateFamilyMembership(Entity *entity);

		void removeEntityInternal(Entity *entity);

		void addEntityInternal(Entity *entity);

		void notifyFamilyListenersAdd(const Family &family, Entity *entity);

		void notifyFamilyListenersRemove(const Family &family, Entity *entity);

		const std::vector<Entity *> *registerFamily(const Family &family);

		void processPendingEntityOperations();

		void processComponentOperations();
		
		template<typename T>
		ComponentPool<T> *getOrCreateComponentPool() {
			auto type = getComponentType<T>();
			if (type >= componentPoolsByType.size())
				componentPoolsByType.resize(type + 1);
			auto *pool = (ComponentPool<T> *)componentPoolsByType[type];
			if (!pool) {
				pool = new ComponentPool<T>(componentPoolInitialSize, componentPoolMaxSize);
				componentPoolsByType[type] = pool;
			}
			return pool;
		}
	};
}