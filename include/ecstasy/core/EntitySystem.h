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
#include "Types.h"

namespace ECS {
	class Engine;
	
	/**
	 * Non-Template base-class for EntitySystem. Extend EntitySystem instead.
	 */
	class EntitySystemBase {
	private:
		friend class Engine;
		bool processing = true;
		Engine *engine = nullptr;

	public:
		/// The unique identifier of this EntitySystem's class
		const SystemType type;
		/// Use this to set the priority of the system. Lower means it'll get executed first.
		int priority;

	private:
		template<typename T> friend class EntitySystem;
		EntitySystemBase(SystemType type, int priority) : type(type), priority(priority) {}
		
	public:
		virtual ~EntitySystemBase() {}

		/**
		 * The update method called every tick.
		 * 
		 * @param deltaTime The time passed since last frame in seconds.
		 */
		virtual void update(float deltaTime) {}

		/// @return Whether or not the system should be processed.
		virtual bool checkProcessing() const {
			return processing;
		}

		/**
		 * Sets whether or not the system should be processed by the Engine.
		 * 
		 * @param processing true to enable, false to disable processing
		 */
		virtual void setProcessing(bool processing) {
			this->processing = processing;
		}
		
		Engine *getEngine() { return engine; }

	protected:
		
		/**
		 * Called when this EntitySystem is added to an Engine.
		 * 
		 * @param engine The Engine this system was added to.
		 */
		virtual void addedToEngine(Engine *engine) {}

		/**
		 * Called when this EntitySystem is removed from an Engine.
		 * 
		 * @param engine The Engine the system was removed from.
		 */
		virtual void removedFromEngine(Engine *engine) {}

	};

	/**
	 * Base class for all systems. An EntitySystem is intended to process entities.
	 * 
	 * @tparam T: The EntitySystem class used to create the type.
	 */
	template<typename T>
	class EntitySystem : public EntitySystemBase {
	public:

		/**
		 * @param priority The priority to execute this system with (lower means higher priority).
		 */
		explicit EntitySystem (int priority=0) : EntitySystemBase(getSystemType<T>(), priority) {}
	};
}

#ifdef USING_ECSTASY
	using ECS::EntitySystemBase;
	using ECS::EntitySystem;
#endif
