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
	 * A simple EntitySystem that iterates over each entity and calls processEntity() for each entity every time the
	 * EntitySystem is updated. This is really just a convenience class as most systems iterate over a list of entities.
	 * 
	 * @tparam T The EntitySystem class used to create the type.
	 */
	template<typename T>
	class IteratingSystem : public EntitySystem<T> {
	private:
		const Family &family;
		const std::vector<Entity *> *entities;

	public:
		/**
		 * Instantiates a system that will iterate over the entities described by the Family, with a specific priority.
		 * 
		 * @param family The family of entities iterated over in this System
		 * @copydetails EntitySystem::EntitySystem()
		 */
		IteratingSystem(const Family &family, int priority = 0) : EntitySystem<T>(priority), family(family) {}

		void update(float deltaTime) override {
			for (auto entity : *entities)
				processEntity(entity, deltaTime);
		}

	protected:
		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(family);
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
	using ECS::IteratingSystem;
#endif
