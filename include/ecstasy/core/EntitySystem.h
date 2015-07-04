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
#include "Types.h"

namespace ECS {
	class Engine;
	
	class EntitySystemBase {
	private:
		//fixme: add Engine ptr
		bool processing = true;

	public:
		const SystemType type;
		/** Use this to set the priority of the system. Lower means it'll get executed first. */
		int priority;

	private:
		template<typename T> friend class EntitySystem;
		EntitySystemBase(SystemType type, int priority) : type(type), priority(priority) {}
		
	public:
		virtual ~EntitySystemBase() {}

		/**
		* Called when this EntitySystem is added to an {@link Engine}.
		* @param engine The {@link Engine} this system was added to.
		*/
		virtual void addedToEngine(Engine *engine) {}

		/**
		* Called when this EntitySystem is removed from an {@link Engine}.
		* @param engine The {@link Engine} the system was removed from.
		*/
		virtual void removedFromEngine(Engine *engine) {}

		/**
		* The update method called every tick.
		* @param deltaTime The time passed since last frame in seconds.
		*/
		virtual void update(float deltaTime) {}

		/** @return Whether or not the system should be processed. */
		virtual bool checkProcessing() const {
			return processing;
		}

		/** Sets whether or not the system should be processed by the {@link Engine}. */
		virtual void setProcessing(bool processing) {
			this->processing = processing;
		}
	};

	/**
	 * Abstract class for processing sets of {@link Entity} objects.
	 * @author Stefan Bachmann
	 */
	template<typename T>
	class EntitySystem : public EntitySystemBase {
	public:

		/**
		 * Initialises the EntitySystem with the priority specified.
		 * @param priority The priority to execute this system with (lower means higher priority).
		 */
		explicit EntitySystem (int priority=0) : EntitySystemBase(getSystemType<T>(), priority) {}
	};
}