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
#include <ecstasy/utils/ComponentFactory.hpp>
#include <ecstasy/utils/EntityFactory.hpp>
#include <ecstasy/utils/Blueprint.hpp>

namespace ecstasy {
	EntityFactory::EntityFactory() {}

	bool EntityFactory::assemble(Entity* entity, const std::string& blueprintname) {
		auto it = entities.find(blueprintname);
		bool success = false;
		if(it != entities.end()) {
			success = true;
			auto blueprint = it->second;
			for(auto& componentBlueprint: blueprint->components) {
				auto factoryIt = componentFactories.find(componentBlueprint->name);
				if(factoryIt == componentFactories.end()
					|| !factoryIt->second->assemble(entity, *componentBlueprint)) {
					success = false;
				}
			}
		}
		return success;
	}
}
