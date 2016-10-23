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

namespace ECS {
	class ComponentBlueprint {
	private:
		friend class EntityFactory;
		std::string name;
		std::map<std::string, std::string> values;

	public:
		ComponentBlueprint(const std::string& name) : name(name) {}

		void set(const std::string& key, const std::string& value);

		bool getBool(const std::string& key, bool defaultValue) const;

		int getInt(const std::string& key, int defaultValue) const;

		float getFloat(const std::string& key, float defaultValue) const;

		const std::string& getString(const std::string& key, const std::string& defaultValue) const;
	};
	
	class EntityBlueprint {
	private:
		friend class EntityFactory;
		std::vector<std::shared_ptr<ComponentBlueprint>> components;

	public:
		void add(std::shared_ptr<ComponentBlueprint> value);
	};
}

#ifdef USING_ECSTASY
	using ECS::ComponentBlueprint;
	using ECS::EntityBlueprint;
#endif
