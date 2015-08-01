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
#include "IntervalSystem.h"
#include "../core/Family.h"
#include "../core/Engine.h"

namespace ECS {
	class Entity;

	/**
	 * A simple EntitySystem that processes a Family of entities not once per frame, but after a given interval.
	 * Entity processing logic should be placed in processEntity().
	 * 
	 * @tparam T: The EntitySystem class used to create the type.
	 */
	template<typename T>
	class IntervalIteratingSystem: public IntervalSystem<T> {
	private:
		const Family &family;
		const std::vector<Entity *> *entities;

	public:
		/**
		 * @param family Represents the collection of family the system should process
		 * @copydetails IntervalSystem::IntervalSystem()
		 */
		IntervalIteratingSystem(const Family &family, float interval, int priority = 0)
			: IntervalSystem<T>(interval, priority), family(family) {}

	protected:
		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(family);
		}

		void updateInterval() override {
			for (auto entity: *entities) {
				processEntity(entity);
			}
		}

	public:
		/// @return A list of entities processed by the system
		const std::vector<Entity *> *getEntities() const {
			return entities;
		}

		/// @return The Family used when the system was created
		const Family &getFamily() const {
			return family;
		}

	protected:
		/**
		 * The user should place the entity processing logic here.
		 * 
		 * @param entity The entity to be processed
		 */
		virtual void processEntity(Entity *entity) = 0;
	};
}

#ifdef USING_ECSTASY
	using ECS::IntervalIteratingSystem;
#endif
