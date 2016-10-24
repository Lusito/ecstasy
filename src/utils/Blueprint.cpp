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
#include <ecstasy/utils/Blueprint.hpp>

namespace ECS {
	void ComponentBlueprint::set(const std::string& key, const std::string& value) {
		values[key] = value;
	}

	bool ComponentBlueprint::getBool(const std::string& key, bool defaultValue) const {
		auto it = values.find(key);
		if(it != values.end()) {
			if(it->second == "true")
				return true;
			if(it->second == "false")
				return false;
			return defaultValue;
		}
		return defaultValue;
	}

	int ComponentBlueprint::getInt(const std::string& key, int defaultValue) const {
		auto it = values.find(key);
		if(it != values.end()) {
			try {
				return std::stoi(it->second);
			} catch(...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	float ComponentBlueprint::getFloat(const std::string& key, float defaultValue) const {
		auto it = values.find(key);
		if(it != values.end()) {
			try {
				return std::stof(it->second);
			} catch(...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	const std::string& ComponentBlueprint::getString(const std::string& key, const std::string& defaultValue) const {
		auto it = values.find(key);
		if(it != values.end())
			return it->second;
		return defaultValue;
	}

	void EntityBlueprint::add(std::shared_ptr<ComponentBlueprint> blueprint) {
		components.push_back(blueprint);
	}
}
