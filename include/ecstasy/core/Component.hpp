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
#include <ecstasy/core/Types.hpp>

namespace ecstasy {
	/// Non-Template base-class for Component. Extend Component instead.
	struct ComponentBase {
		/// The unique identifier of this Component's class
		const ComponentType type;
		const uint32_t memorySize;
		virtual ~ComponentBase() {}

	private:
		template<typename T> friend struct Component;
		// Private Constructor so nobody derives from this
		explicit ComponentBase(ComponentType type, uint32_t memorySize)
			: type(type), memorySize(memorySize) {}
	};
	
	/**
	 * Base class for all components. A Component is intended as a data holder
	 * and provides data to be processed in an EntitySystem.
	 * 
	 * @tparam T: The Component class used to create the type.
	 */
	template<typename T>
	struct Component : public ComponentBase {
		Component() : ComponentBase(getComponentType<T>(), sizeof(T)) {}
	};
}

#ifdef USING_ECSTASY
	using ecstasy::ComponentBase;
	using ecstasy::Component;
#endif
