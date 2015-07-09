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
	class ComponentOperation: public Poolable {
	public:
		enum class Type {
			Add,
			Remove,
			RemoveAll
		};

	public:
		Type type;
		Entity *entity;
		ComponentBase *component;
		ComponentType componentType;

		void makeAdd(Entity *entity, ComponentBase *component);
		void makeRemove(Entity *entity, ComponentType componentType);
		void makeRemoveAll(Entity* entity);
		void reset() override;
	};
	
	class ComponentOperationHandler {
	private:
		Engine &engine;

	public:
		explicit ComponentOperationHandler(Engine &engine) : engine(engine) {}

		bool isActive();

		void add(Entity *entity, ComponentBase *component);

		void remove(Entity *entity, ComponentType componentType);

		void removeAll(Entity *entity);
	};

/// \endcond
}
