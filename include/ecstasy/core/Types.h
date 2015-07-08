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
#include "../utils/Bits.h"


namespace ECS {
	/**
	 * Gets a unique id for a typed integral
	 * 
	 * @tparam T The type to get an id for.
	 */
	template <typename T>
	T getUniqueTypeId() {
		static T type = 0;
		return type++;
	}
	
	/// Uniquely identifies a Component sub-class.
	typedef uint32_t ComponentType;
	
	/**
	 * Get a unique index for a specified Component class.
	 * 
	 * @tparam T The Component class
     * @return A unique identifier
     */
	template <typename T>
	ComponentType getComponentType() {
		static ComponentType type(getUniqueTypeId<ComponentType>());
		return type;
	}

	/// Uniquely identifies an EntitySystem sub-class.
	typedef uint32_t SystemType;
	
	/**
	 * Get a unique index for a specified EntitySystem class.
	 * 
	 * @tparam T The EntitySystem class
     * @return A unique identifier
     */
	template <typename T>
	SystemType getSystemType() {
		static SystemType type(getUniqueTypeId<SystemType>());
		return type;
	}
}