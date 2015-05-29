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
#include <vector>
#include "../core/Family.h"
#include "../core/Engine.h"
#include "../core/EntityListener.h"

namespace ECS {
	class Entity;

	// Comparator example:
//	bool compareSystemss(Entity *a, Entity *b) {
//		return a->priority < b->priority;
//	}

	/**
	 * A simple EntitySystem that processes each entity of a given family in the order specified by a comparator and calls
	 * processEntity() for each entity every time the EntitySystem is updated. This is really just a convenience class as rendering
	 * systems tend to iterate over a list of entities in a sorted manner. Adding entities will cause the entity list to be resorted.
	 * Call forceSort() if you changed your sorting criteria.
	 * @author Santo Pfingsten
	 */
	template<typename T, typename C>
	class SortedIteratingSystem : public EntitySystem<T>, public EntityListener {
	private:
		Family &family;
		std::vector<Entity *> sortedEntities;
		bool shouldSort;
		C comparator;

	public:
		/**
		* Instantiates a system that will iterate over the entities described by the Family, with a specific priority.
		* @param family The family of entities iterated over in this System
		* @param comparator The comparator to sort the entities
		* @param priority The priority to execute this system with (lower means higher priority)
		*/
		SortedIteratingSystem(Family &family, C comparator, int priority=0) : EntitySystem<T>(priority) , family(family), comparator(comparator) {}

		/**
		* Call this if the sorting criteria have changed. The actual sorting will be delayed until the entities are processed.
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
		
	public:
		void addedToEngine(Engine *engine) override {
			const std::vector<Entity *> *newEntities = engine->getEntitiesFor(family);
			sortedEntities.clear();
			if (!newEntities->empty()) {
				for (auto entity : *newEntities) {
					sortedEntities.push_back(entity);
				}
				std::sort(sortedEntities.begin(), sortedEntities.end(), comparator);
			}
			shouldSort = false;
			engine->addEntityListener(family, this);
		}

		void removedFromEngine(Engine *engine) override {
			engine->removeEntityListener(this);
			sortedEntities.clear();
			shouldSort = false;
		}

		void entityAdded(Entity *entity) override {
			sortedEntities.push_back(entity);
			shouldSort = true;
		}

		void entityRemoved(Entity *entity) override {
			auto it = std::find(sortedEntities.begin(), sortedEntities.end(), entity);
			if (it != sortedEntities.end()) {
				sortedEntities.erase(it);
				shouldSort = true;
			}
		}

		void update(float deltaTime) override {
			sort();
			for (auto entity : sortedEntities) {
				processEntity(entity, deltaTime);
			}
		}

		/**
		* @return set of entities processed by the system
		*/
		const std::vector<Entity *> *getEntities() {
			sort();
			return &sortedEntities;
		}

		/**
		* @return the Family used when the system was created
		*/
		Family &getFamily() {
			return family;
		}

		/**
		* This method is called on every entity on every update call of the EntitySystem. Override this to implement your system's
		* specific processing.
		* @param entity The current Entity being processed
		* @param deltaTime The delta time between the last and current frame
		*/
	protected:
		virtual void processEntity(Entity *entity, float deltaTime) = 0;
	};
}