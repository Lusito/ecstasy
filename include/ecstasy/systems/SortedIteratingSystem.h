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
#include <vector>
#include "../core/Family.h"
#include "../core/Engine.h"

namespace ECS {
	class Entity;

	/**
	 * A simple EntitySystem that processes each Entity of a given Family in the order specified by a comparator and
	 * calls processEntity() for each Entity every time the EntitySystem is updated. This is really just a convenience
	 * class as rendering systems tend to iterate over a list of entities in a sorted manner. Adding entities will cause
	 * the entity list to be resorted. Call forceSort() if you changed your sorting criteria.
	 * 
	 * @tparam T The EntitySystem class used to create the type.
	 * @tparam C The comparator type
	 */
	template<typename T, typename C>
	class SortedIteratingSystem : public EntitySystem<T> {
	private:
		const Family &family;
		std::vector<Entity *> sortedEntities;
		bool shouldSort;
		C comparator;
		Signal11::ConnectionScope scope;

	public:
		/**
		 * Instantiates a system that will iterate over the entities described by the Family, with a specific priority.
		 * 
		 * @param family The family of entities iterated over in this System
		 * @param comparator The comparator to sort the entities
		 * @copydetails EntitySystem::EntitySystem()
		 */
		SortedIteratingSystem(const Family &family, C comparator, int priority=0)
			: EntitySystem<T>(priority) , family(family), comparator(comparator) {}

		/**
		 * Call this if the sorting criteria have changed.
		 * The actual sorting will be delayed until the entities are processed.
		 */
		void forceSort() {
			shouldSort = true;
		}

	private:
		void sort() {
			if (shouldSort) {
				std::sort(sortedEntities.begin(), sortedEntities.end(), comparator);
				shouldSort = false;
			}
		}

		void entityAdded(Entity *entity) {
			sortedEntities.push_back(entity);
			shouldSort = true;
		}

		void entityRemoved(Entity *entity) {
			auto it = std::find(sortedEntities.begin(), sortedEntities.end(), entity);
			if (it != sortedEntities.end()) {
				sortedEntities.erase(it);
				shouldSort = true;
			}
		}

	protected:
		void addedToEngine(Engine *engine) override {
			auto *newEntities = engine->getEntitiesFor(family);
			sortedEntities.clear();
			if (!newEntities->empty()) {
				for (auto entity : *newEntities) {
					sortedEntities.push_back(entity);
				}
				std::sort(sortedEntities.begin(), sortedEntities.end(), comparator);
			}
			shouldSort = false;
			scope += engine->getEntityAddedSignal(family).connect(this, &SortedIteratingSystem::entityAdded);
			scope += engine->getEntityRemovedSignal(family).connect(this, &SortedIteratingSystem::entityRemoved);
		}

		void removedFromEngine(Engine *engine) override {
			scope.removeAll();
			sortedEntities.clear();
			shouldSort = false;
		}

	public:
		void update(float deltaTime) override {
			sort();
			for (auto entity : sortedEntities) {
				processEntity(entity, deltaTime);
			}
		}

		/**
		 * @return set of entities processed by the system
		 */
		const std::vector<Entity *> *getEntities() const {
			sort();
			return &sortedEntities;
		}

		/// @return The Family used when the system was created
		const Family &getFamily() const {
			return family;
		}

	protected:
		/**
		 * This method is called on every entity on every update call of the EntitySystem.
		 * Override this to implement your system's specific processing.
		 * 
		 * @param entity The current Entity being processed
		 * @param deltaTime The delta time between the last and current frame
		 */
		virtual void processEntity(Entity *entity, float deltaTime) = 0;
	};
}

#ifdef USING_ECSTASY
	using ECS::SortedIteratingSystem;
#endif
