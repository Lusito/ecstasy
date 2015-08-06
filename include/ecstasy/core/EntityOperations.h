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
#include "../utils/Pool.h"
#include "Types.h"

namespace ECS {
	class Entity;
	class Engine;
	struct ComponentBase;
	
/// \cond HIDDEN_SYMBOLS
	enum class OperationType {
		Add,
		Remove,
		RemoveAll
	};
	
	template<typename T>
	struct BaseOperation: public Poolable {
		T *nextOperation = nullptr;
		OperationType type;
	};

	struct EntityOperation: public BaseOperation<EntityOperation> {

		Entity *entity = nullptr;

		void reset() override {
			nextOperation = nullptr;
			entity = nullptr;
		}
	};
	
	struct ComponentOperation: public BaseOperation<ComponentOperation> {
		Entity *entity = nullptr;
		ComponentBase *component = nullptr;
		ComponentType componentType;
		
		void reset() override {
			nextOperation = nullptr;
			entity = nullptr;
			component = nullptr;
		}

		void makeAdd(Entity *entity, ComponentBase *component);
		void makeRemove(Entity *entity, ComponentType componentType);
		void makeRemoveAll(Entity* entity);
	};
	
	template<typename T>
	class BaseOperationHandler {
	protected:
		Engine &engine;
		Pool<T> pool;
		T *nextOperation = nullptr;
		T *lastOperation = nullptr;

	public:
		explicit BaseOperationHandler(Engine &engine) : engine(engine) {}

		virtual bool isActive() = 0;
		void process() {
			while(nextOperation) {
				auto *operation = nextOperation;
				switch(operation->type) {
				case OperationType::Add: onAdd(operation); break;
				case OperationType::Remove: onRemove(operation); break;
				case OperationType::RemoveAll: onRemoveAll(operation); break;
				default:
					throw std::runtime_error("Unexpected EntityOperation type");
				}

				nextOperation = operation->nextOperation;
				pool.free(operation);
			}
			nextOperation = nullptr;
			lastOperation = nullptr;
		}
		
		/**
		 * Removes all free operations from the pool to free up memory.
		 */
		void clearPools() {
			pool.clear();
		}
		
	protected:
		void schedule(T *operation) {
			if(nextOperation)
				lastOperation->nextOperation = operation;
			else
				nextOperation = operation;
			lastOperation = operation;
		}
		
		virtual void onAdd(T *operation) = 0;
		virtual void onRemove(T *operation) = 0;
		virtual void onRemoveAll(T *operation) = 0;
	};
	
	class EntityOperationHandler : public BaseOperationHandler<EntityOperation> {
	public:
		explicit EntityOperationHandler(Engine &engine) : BaseOperationHandler(engine) {}

		bool isActive() override;
		
		void add(Entity *entity) {
			auto *operation = pool.obtain();
			operation->type = OperationType::Add;
			operation->entity = entity;
		
			schedule(operation);
		}

		void remove(Entity *entity) {
			auto *operation = pool.obtain();
			operation->type = OperationType::Remove;
			operation->entity = entity;
			schedule(operation);
		}

		void removeAll() {
			auto *operation = pool.obtain();
			operation->type = OperationType::RemoveAll;
			schedule(operation);
		}
		
	protected:
		void onAdd(EntityOperation *operation) override;
		void onRemove(EntityOperation *operation) override;
		void onRemoveAll(EntityOperation *operation) override;
	};
	
	class ComponentOperationHandler : public BaseOperationHandler<ComponentOperation> {
	public:
		explicit ComponentOperationHandler(Engine &engine) : BaseOperationHandler(engine) {}

		bool isActive() override;
		
		void add(Entity *entity, ComponentBase *component) {
			auto *operation = pool.obtain();
			operation->type = OperationType::Add;
			operation->entity = entity;
			operation->component = component;
			operation->componentType = 0;
		
			schedule(operation);
		}

		void remove(Entity *entity, ComponentType componentType) {
			auto *operation = pool.obtain();
			operation->type = OperationType::Remove;
			operation->entity = entity;
			operation->componentType = componentType;
			schedule(operation);
		}

		void removeAll(Entity *entity) {
			auto *operation = pool.obtain();
			operation->type = OperationType::RemoveAll;
			operation->entity = entity;
			schedule(operation);
		}
		
	protected:
		void onAdd(ComponentOperation *operation) override;
		void onRemove(ComponentOperation *operation) override;
		void onRemoveAll(ComponentOperation *operation) override;
	};

/// \endcond
}
