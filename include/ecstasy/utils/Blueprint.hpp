#pragma once
/*******************************************************************************
 * Copyright 2011 See AUTHORS file.
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

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace ecstasy {
	/**
	 * Stores the name of a component and key/value pairs to construct the component.
	 * See EntityFactory.
	 */
	class ComponentBlueprint {
	private:
		friend class EntityFactory;
		std::string name;
		std::map<std::string, std::string> values;

	public:
		/**
		 * Creates a new blueprint with the specified component name
		 *
		 * @param name the name of the component.
		 */
		ComponentBlueprint(const std::string& name) : name(name) {}
		ComponentBlueprint(const ComponentBlueprint&) = delete;

		/**
		 * Set a key/value pair
		 *
		 * @param key the key
		 * @param value the value
		 */
		void set(const std::string& key, const std::string& value);

		/**
		 * Get a boolean value
		 *
		 * @param key the key
		 * @param defaultValue the value to return if no value exists for key.
		 * @return The corresponding value or @a defaultValue if none exists.
		 */
		bool getBool(const std::string& key, bool defaultValue) const;

		/**
		 * Get an integer value
		 *
		 * @param key the key
		 * @param defaultValue the value to return if no value exists for key.
		 * @return The corresponding value or @a defaultValue if none exists.
		 */
		int getInt(const std::string& key, int defaultValue) const;

		/**
		 * Get a float value
		 *
		 * @param key the key
		 * @param defaultValue the value to return if no value exists for key.
		 * @return The corresponding value or @a defaultValue if none exists.
		 */
		float getFloat(const std::string& key, float defaultValue) const;

		/**
		 * Get a string value
		 *
		 * @param key the key
		 * @param defaultValue the value to return if no value exists for key.
		 * @return The corresponding value or @a defaultValue if none exists.
		 */
		const std::string& getString(const std::string& key, const std::string& defaultValue) const;
	};

	/**
	 * Stores a list of {@link ComponentBlueprint}s needed to construct an Entity.
	 * See EntityFactory.
	 */
	class EntityBlueprint {
	private:
		friend class EntityFactory;
		std::vector<std::shared_ptr<ComponentBlueprint>> components;

	public:
		EntityBlueprint() {}
		EntityBlueprint(const EntityBlueprint&) = delete;
		/// @param blueprint shared_ptr to a ComponentBlueprint.
		void add(std::shared_ptr<ComponentBlueprint> blueprint);
	};
}

#ifdef USING_ECSTASY
	using ecstasy::ComponentBlueprint;
	using ecstasy::EntityBlueprint;
#endif
