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

#include "Engine.h"
#include "Component.h"

namespace ECS {
	/**
	 * Supports {@link Entity} and {@link Component} pooling. This improves performance in environments where creating/deleting
	 * entities is frequent as it greatly reduces memory allocation.
	 * <ul>
	 * <li>Create entities using {@link #createEntity()}</li>
	 * <li>Create components using {@link #createComponent(Class)}</li>
	 * <li>Components should implement the {@link Poolable} interface when in need to reset its state upon removal</li>
	 * </ul>
	 * @author David Saltares
	 */
	class PooledEngine : public Engine {
	private:
		/** Component Pools */
		class ComponentPoolBase {
		public:
			virtual ~ComponentPoolBase() {}
			virtual void freeComponent(ComponentBase *object) = 0;
		};
		
		std::vector<ComponentPoolBase *> componentPoolsByType;
		
		template<typename T>
		class ComponentPool : public ComponentPoolBase, public ReflectionPool<T> {
		public:
			void freeComponent(ComponentBase *object) {
				ReflectionPool<T>::free((T*)object);
			}
		};
		
		template<typename T>
		ComponentPool<T> *getOrCreateComponentPool() {
			auto type = getComponentType<T>();
			if (type >= componentPoolsByType.size())
				componentPoolsByType.resize(type + 1);
			ComponentPool<T> *pool = (ComponentPool<T> *)componentPoolsByType[type];
			if (!pool) {
				pool = new ComponentPool<T>();
				componentPoolsByType[type] = pool;
			}
			return pool;
		}
		
		class PooledEntity: public Entity, public Poolable {
		public:
			PooledEngine *engine = nullptr;

		protected:
			ComponentBase *removeInternal(ComponentType type) override;

		public:
			void reset() override;
		};
		
		/** Entity Pool */
		class EntityPool : public Pool<PooledEntity> {
		public:
			EntityPool(int initialSize, int maxSize) : Pool<PooledEntity>(initialSize, maxSize) {}

		protected:
			PooledEntity *newObject() override {
				return new PooledEntity();
			}
		};
		EntityPool entityPool;

	public:
		/**
		 * Creates a new PooledEngine with a maximum of 100 entities and 100 components of each type. Use
		 * {@link #PooledEngine(int, int, int, int)} to configure the entity and component pools.
		 */
		PooledEngine () : PooledEngine(10, 100, 10, 100) {}

		/**
		 * Creates new PooledEngine with the specified pools size configurations.
		 * @param entityPoolInitialSize initial number of pre-allocated entities.
		 * @param entityPoolMaxSize maximum number of pooled entities.
		 * @param componentPoolInitialSize initial size for each component type pool.
		 * @param componentPoolMaxSize maximum size for each component type pool.
		 */
		PooledEngine (int entityPoolInitialSize, int entityPoolMaxSize, int componentPoolInitialSize, int componentPoolMaxSize) //fixme: pass parameters to component pools
		: entityPool(entityPoolInitialSize, entityPoolMaxSize) {
		}

		~PooledEngine() {
			clearPools();
		}

		/** @return Clean {@link Entity} from the Engine pool. In order to add it to the {@link Engine}, use {@link #addEntity(Entity)}. */
		Entity *createEntity();

		/**
		 * Retrieves a new {@link Component} from the {@link Engine} pool. It will be placed back in the pool whenever it's removed
		 * from an {@link Entity} or the {@link Entity} itself it's removed.
		 */
		template<typename T>
		T *createComponent() {
			return getOrCreateComponentPool<T>()->obtain();
		}
		
		void free(ComponentBase *component);

		/**
		 * Removes all free entities and components from their pools. Although this will likely result in garbage collection, it will
		 * free up memory.
		 */
		void clearPools();

	protected:
		void removeEntityInternal(Entity *entity) override;
	};
}