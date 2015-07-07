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
#include <ecstasy/core/Entity.h>
#include <ecstasy/core/Component.h>
#include <ecstasy/core/Engine.h>

namespace ECS {
	Entity &Entity::add (ComponentBase *component) {
		if (componentOperationHandler != nullptr && componentOperationHandler->isActive())
			componentOperationHandler->add(this, component);
		else
			addInternal(component);
		return *this;
	}

	void Entity::removeAll() {
		if (componentOperationHandler != nullptr && componentOperationHandler->isActive())
			componentOperationHandler->removeAll(this);
		else
			removeAllInternal();
	}

	void Entity::addInternal (ComponentBase *component) {
		auto type = component->type;
		auto *oldComponent = getComponent(type);
		if (component == oldComponent)
			return;

		if (oldComponent != nullptr)
			removeInternal(type);

		if (type >= componentsByType.size())
			componentsByType.resize(type + 1);

		componentsByType[type] = component;
		components.push_back(component);

		componentBits.set(type);

		engine->componentAdded.emit(this, component);
	}

	ComponentBase *Entity::removeInternal(ComponentType type) {
		auto component = componentsByType[type];
		if (component) {
			componentsByType[type] = nullptr;
			
			components.erase(std::remove(components.begin(), components.end(), component), components.end());
			componentBits.clear(type);

			engine->componentRemoved.emit(this, component);
			
			auto pool = engine->componentPoolsByType[type];
			pool->freeComponent(component);
		}
		return component;
	}

	void Entity::removeAllInternal() {
		while (!components.empty())
			removeInternal(components.front()->type);
	}

	void Entity::reset() {
		removeAll();
		uuid = 0;
		flags = 0;
		scheduledForRemoval = false;
		engine = nullptr;
	}
}