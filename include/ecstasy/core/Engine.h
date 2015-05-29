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
#include "../utils/SnapshotVector.h"
#include "ComponentOperationHandler.h"
#include "Family.h"
#include "../signals/Signal.h"
#include "../utils/Pool.h"
#include <stdint.h>
#include <vector>
#include <map>

namespace Ashley {
	class EntityListener;
	
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
		
		class ComponentListener: public Receiver<Entity *> {
		private:
			Engine &engine;

		public:
			explicit ComponentListener(Engine &engine) : engine(engine) {}

			void receive(Signal<Entity *> &signal, Entity *object) override {
				engine.updateFamilyMembership(object);
			}
		};

		class ComponentOperationPool: public Pool<ComponentOperation> {
		protected:
			ComponentOperation *newObject() override {
				return new ComponentOperation();
			}
		};

		class EntityOperation: public Poolable {
		public:
			enum class Type {
				Add,
				Remove,
				RemoveAll
			};

			Type type;
			Entity *entity;

			void reset() override {
				entity = nullptr;
			}
		};

		class EntityOperationPool: public Pool<EntityOperation> {
		protected:
			EntityOperation *newObject() override {
				return new EntityOperation();
			}
		};

		std::vector<Entity *> entities;
		std::map<uint64_t, Entity *> entitiesById;

		std::vector<EntityOperation *> entityOperations;
		EntityOperationPool entityOperationPool;

		std::vector<EntitySystemBase *> systems;
		std::map<SystemType, EntitySystemBase *> systemsByType;

		std::map<Family *, std::vector<Entity *>> entitiesByFamily;

		SnapshotVector<EntityListener *> entityListeners;
		std::map<Family *,SnapshotVector<EntityListener *>> familyListeners;

		ComponentListener componentAdded;
		ComponentListener componentRemoved;

		bool updating = false;

		bool notifying = false;
		uint64_t nextEntityId = 1;

		/** Mechanism to delay component addition/removal to avoid affecting system processing */
		ComponentOperationPool componentOperationsPool;
		std::vector<ComponentOperation *> componentOperations;
		ComponentOperationHandler componentOperationHandler;


		uint64_t obtainEntityId() {
			return nextEntityId++;
		}
		
	public:
		Engine() : componentAdded(*this), componentRemoved(*this), componentOperationHandler(*this) {}
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

		Entity *getEntity(uint64_t id);

		const std::vector<Entity *> *getEntities() {
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
		T *getSystem() {
			auto it = systemsByType.find(getSystemType<T>());
			if(it == systemsByType.end())
				return nullptr;
			return (T *) it->second;
		}

		/**
		 * @return immutable array of all entity systems managed by the {@link Engine}.
		 */
		const std::vector<EntitySystemBase *> &getSystems() {
			return systems;
		}

		/**
		 * Returns immutable collection of entities for the specified {@link Family}. Will return the same instance every time.
		 */
		const std::vector<Entity *> *getEntitiesFor(Family &family){
			return registerFamily(family);
		}

		/**
		 * Adds an {@link EntityListener}.
		 *
		 * The listener will be notified every time an entity is added/removed to/from the engine.
		 */
		void addEntityListener(EntityListener *listener) {
			entityListeners.add(listener);
		}

		/**
		 * Adds an {@link EntityListener} for a specific {@link Family}.
		 *
		 * The listener will be notified every time an entity is added/removed to/from the given family.
		 */
		void addEntityListener(Family &family, EntityListener *listener);

		/**
		 * Removes an {@link EntityListener}
		 */
		void removeEntityListener(EntityListener *listener);

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
		void notifyFamilyListenersAdd(Family &family, Entity *entity);

		void notifyFamilyListenersRemove(Family &family, Entity *entity);

		const std::vector<Entity *> *registerFamily(Family &family);

		void processPendingEntityOperations();

		void processComponentOperations();
	};
}