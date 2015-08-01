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

#include <stdint.h>
#include "ComponentOperations.h"

namespace ECS {
	/**
	 * Simple containers of {@link Component}s that give them "data".
	 * The component's data is then processed by {@link EntitySystem}s.
	 */
	class Entity: public Poolable {
		friend class Family;
		friend class ComponentOperationHandler;
		friend class Engine;
		friend class Pool<Entity>;
	public:
		/// A flag that can be used to bit mask this entity. Up to the user to manage.
		uint32_t flags = 0;

	private:
		uint64_t uuid = 0;
		bool scheduledForRemoval = false;
		ComponentOperationHandler *componentOperationHandler = nullptr;

		std::vector<ComponentBase *> componentsByType;
		std::vector<ComponentBase *> components;
		Bits componentBits;
		Bits familyBits;
		Engine *engine = nullptr;

		Entity() {}
		
	public:
		~Entity() { removeAll(); }
		void reset() override;

		/// @return The Entity's unique id.
		uint64_t getId () const { return uuid; }
		
		/// @return true if the entity is valid (added to the engine).
		bool isValid () const { return uuid > 0; }

		/// @return true if the entity is scheduled to be removed
		bool isScheduledForRemoval () const { return scheduledForRemoval; }

		/**
		 * Adds a Component to this Entity. If a Component of the same type already exists, it'll be replaced.
		 * 
		 * @param component The Component to add
		 * @return The Entity for easy chaining
		 */
		Entity &add(ComponentBase *component);

		/**
		 * Removes the Component of the specified type. Since there is only ever one Component of one type, we don't
		 * need an instance reference.
		 * 
		 * @tparam T The Component class
		 */
		template<typename T>
		void remove () {
			auto type = getComponentType<T>();
			if (componentOperationHandler != nullptr && componentOperationHandler->isActive())
				componentOperationHandler->remove(this, type);
			else
				removeInternal(type);
		}

		/// Removes all the {@link Component}s from the Entity.
		void removeAll();

		/// @return A list with all the {@link Component}s of this Entity.
		const std::vector<ComponentBase *> &getAll () const {
			return components;
		}

		/**
		 * Retrieve a Component from this Entity by class.
		 * 
		 * @tparam T The Component class
		 * @return The instance of the specified Component attached to this Entity, or null if no such Component exists.
		 */
		template<typename T>
		T *get() const {
			return (T *)getComponent(getComponentType<T>());
		}

		/**
		 * @tparam T The Component class
		 * @return Whether or not the Entity has a Component for the specified class.
		 */
		template<typename T>
		bool has() const {
			return componentBits.get(getComponentType<T>());
		}
	private:
		/// @return The Component object for the specified class, null if the Entity does not have any components for that class.
		ComponentBase *getComponent(ComponentType componentType) const {
			if (componentType >= componentsByType.size())
				return nullptr;
			return componentsByType[componentType];
		}
		
		void addInternal (ComponentBase *component);
		ComponentBase *removeInternal(ComponentType type);
		void removeAllInternal();

	public:
		/// @return This Entity's Component bits, describing all the {@link Component}s it contains.
		const Bits &getComponentBits() const {
			return componentBits;
		}

		/// @return This Entity's Family bits, describing all the {@link EntitySystem}s it currently is being processed by.
		const Bits &getFamilyBits() const {
			return familyBits;
		}

	public:
		/// @return true if the entities are equal
		bool operator ==(const Entity &other) const {
			return this == &other;
		}

		/// @return true if the entities are unequal
		bool operator !=(const Entity &other) const {
			return this != &other;
		}
	};
}

#ifdef USING_ECSTASY
	using ECS::Entity;
#endif
