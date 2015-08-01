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
#include "../core/EntitySystem.h"

namespace ECS {
	class Entity;

	/**
	 * A simple EntitySystem that does not run its update logic every call to update(float), but after a
	 * given interval. The actual logic should be placed in updateInterval().
	 * 
	 * @tparam T: The EntitySystem class used to create the type.
	 */
	template<typename T>
	class IntervalSystem: public EntitySystem<T> {
	private:
		float interval;
		float accumulator = 0;

	public:
		/**
		 * @param interval time in seconds between calls to updateInterval().
		 * @copydetails EntitySystem::EntitySystem()
		 */
		explicit IntervalSystem(float interval, int priority = 0) : EntitySystem<T>(priority), interval(interval) {}

		void update(float deltaTime) override {
			accumulator += deltaTime;

			while (accumulator >= interval) {
				accumulator -= interval;
				updateInterval();
			}
		}

	protected:
		/**
		 * The processing logic of the system should be placed here.
		 */
		virtual void updateInterval() = 0;
	};
}

#ifdef USING_ECSTASY
	using ECS::IntervalSystem;
#endif
