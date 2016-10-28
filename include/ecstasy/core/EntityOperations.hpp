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
#include <stdexcept>
#include <ecstasy/core/Types.hpp>
#include <ecstasy/utils/MemoryManager.hpp>

namespace ecstasy {
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
	struct BaseOperation {
		T* nextOperation = nullptr;
		OperationType type;
	};

	struct EntityOperation: public BaseOperation<EntityOperation> {
		Entity* entity = nullptr;
	};
	
	struct ComponentOperation: public BaseOperation<ComponentOperation> {
		Entity* entity = nullptr;
		ComponentBase* component = nullptr;
		ComponentType componentType;

		void makeAdd(Entity* entity, ComponentBase* component);
		void makeRemove(Entity* entity, ComponentType componentType);
		void makeRemoveAll(Entity* entity);
	};
	
	template<typename T>
	class BaseOperationHandler {
	protected:
		Engine& engine;
		MemoryManager* memoryManager;
		T* nextOperation = nullptr;
		T* lastOperation = nullptr;

	public:
		explicit BaseOperationHandler(Engine& engine, MemoryManager* memoryManager) : engine(engine), memoryManager(memoryManager) {}

		T *createOperation() {
			auto memory = memoryManager->allocate(sizeof(T));
			return new(memory) T();
		}

		virtual bool isActive() = 0;
		void process() {
			while(nextOperation) {
				auto operation = nextOperation;
				switch(operation->type) {
				case OperationType::Add: onAdd(operation); break;
				case OperationType::Remove: onRemove(operation); break;
				case OperationType::RemoveAll: onRemoveAll(operation); break;
				default:
					throw std::runtime_error("Unexpected EntityOperation type");
				}

				nextOperation = operation->nextOperation;
				operation->~T();
				memoryManager->free(sizeof(T), operation);
			}
			nextOperation = nullptr;
			lastOperation = nullptr;
		}

	protected:
		void schedule(T* operation) {
			if(nextOperation)
				lastOperation->nextOperation = operation;
			else
				nextOperation = operation;
			lastOperation = operation;
		}

		virtual void onAdd(T* operation) = 0;
		virtual void onRemove(T* operation) = 0;
		virtual void onRemoveAll(T* operation) = 0;
	};
	
	class EntityOperationHandler : public BaseOperationHandler<EntityOperation> {
	public:
		explicit EntityOperationHandler(Engine& engine, MemoryManager* memoryManager)
			: BaseOperationHandler(engine, memoryManager) {}

		bool isActive() override;

		void add(Entity* entity) {
			auto operation = createOperation();
			operation->type = OperationType::Add;
			operation->entity = entity;
		
			schedule(operation);
		}

		void remove(Entity* entity) {
			auto operation = createOperation();
			operation->type = OperationType::Remove;
			operation->entity = entity;
			schedule(operation);
		}

		void removeAll() {
			auto operation = createOperation();
			operation->type = OperationType::RemoveAll;
			schedule(operation);
		}

	protected:
		void onAdd(EntityOperation* operation) override;
		void onRemove(EntityOperation* operation) override;
		void onRemoveAll(EntityOperation* operation) override;
	};
	
	class ComponentOperationHandler : public BaseOperationHandler<ComponentOperation> {
	public:
		explicit ComponentOperationHandler(Engine& engine, MemoryManager* memoryManager)
			: BaseOperationHandler(engine, memoryManager) {}

		bool isActive() override;

		void add(Entity* entity, ComponentBase* component) {
			auto operation = createOperation();
			operation->type = OperationType::Add;
			operation->entity = entity;
			operation->component = component;
			operation->componentType = 0;
		
			schedule(operation);
		}

		void remove(Entity* entity, ComponentType componentType) {
			auto operation = createOperation();
			operation->type = OperationType::Remove;
			operation->entity = entity;
			operation->componentType = componentType;
			schedule(operation);
		}

		void removeAll(Entity* entity) {
			auto operation = createOperation();
			operation->type = OperationType::RemoveAll;
			operation->entity = entity;
			schedule(operation);
		}
		
	protected:
		void onAdd(ComponentOperation* operation) override;
		void onRemove(ComponentOperation* operation) override;
		void onRemoveAll(ComponentOperation* operation) override;
	};

/// \endcond
}
