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

#include <stdint.h>
#include "ComponentOperations.h"

namespace ECS {
	/**
	 * Simple containers of {@link Component}s that give them "data". The component's data is then processed by {@link EntitySystem}s.
	 * @author Stefan Bachmann
	 */
	class Entity: public Poolable {
		friend class Family;
		friend class ComponentOperationHandler;
		friend class Engine;
		friend class EntityPool;
	public:
		/** A flag that can be used to bit mask this entity. Up to the user to manage. */
		int flags = 0;

	protected:
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

		/** @return The Entity's unique id. */
		uint64_t getId () const { return uuid; }
		
		/** @return true if the entity is valid (added to the engine). */
		bool isValid () const { return uuid > 0; }

		/** @return true if the entity is scheduled to be removed */
		bool isScheduledForRemoval () const { return scheduledForRemoval; }

		/**
		 * Adds a {@link Component} to this Entity. If a {@link Component} of the same type already exists, it'll be replaced.
		 * @return The Entity for easy chaining
		 */
		Entity &add (ComponentBase *component);

		/**
		 * Removes the {@link Component} of the specified type. Since there is only ever one component of one type, we don't need an
		 * instance reference.
		 */
		template<typename T>
		void remove () {
			auto type = getComponentType<T>();
			if (componentOperationHandler != nullptr && componentOperationHandler->isActive())
				componentOperationHandler->remove(this, type);
			else
				removeInternal(type);
		}

		/** Removes all the {@link Component}'s from the Entity. */
		void removeAll();

		/** @return immutable collection with all the Entity {@link Component}s. */
		const std::vector<ComponentBase *> &getAll () const {
			return components;
		}

		/**
		 * Retrieve a component from this {@link Entity} by class.
		 * @return the instance of the specified {@link Component} attached to this {@link Entity}, or null if no such
		 *         {@link Component} exists.
		 */
		template<typename T>
		T *get() const {
			return (T *)getComponent(getComponentType<T>());
		}

		/**
		 * @return Whether or not the Entity has a {@link Component} for the specified class.
		 */
		template<typename T>
		bool has() const {
			return componentBits.get(getComponentType<T>());
		}
	private:
		/**
		 * Internal use.
		 * @return The {@link Component} object for the specified class, null if the Entity does not have any components for that class.
		 */
		ComponentBase *getComponent(ComponentType componentType) const {
			if (componentType >= componentsByType.size())
				return nullptr;
			return componentsByType[componentType];
		}

	public:
		/**
		 * @return This Entity's component bits, describing all the {@link Component}s it contains.
		 */
		const Bits &getComponentBits() const {
			return componentBits;
		}

		/** @return This Entity's {@link Family} bits, describing all the {@link EntitySystem}s it currently is being processed by. */
		const Bits &getFamilyBits() const {
			return familyBits;
		}
		
	protected:
		void addInternal (ComponentBase *component);
		ComponentBase *removeInternal(ComponentType type);
		void removeAllInternal();

	public:
		bool operator ==(const Entity &other) const {
			if (this == &other) return true;
			return uuid == other.uuid;
		}

		bool operator !=(const Entity &other) const {
			if (this == &other) return false;
			return uuid != other.uuid;
		}
	};
}