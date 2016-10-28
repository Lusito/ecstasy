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

#include <unordered_map>
#include <string>
#include <memory>

namespace ecstasy {
	class Entity;
	class EntityBlueprint;
	class ComponentFactory;

	/**
	 * A factory to create {@link Entity entities} from blueprints.
	 */
	class EntityFactory {
	private:
		std::unordered_map<std::string, std::unique_ptr<ComponentFactory>> componentFactories;
		std::unordered_map<std::string, std::shared_ptr<EntityBlueprint>> entities;

	public:
		/// Default constructor
		EntityFactory();
		EntityFactory(const EntityFactory&) = delete;

		/**
		 * Add a component factory
		 *
		 * @tparam T the {@link ComponentFactory} class
		 * @param name the name used to identify a {@link Component}
		 * @param args the arguments to pass to the {@link ComponentFactory} constructor
		 */
		template <typename T, typename ... Args>//fixme: make sure it extends ComponentFactory
		void addComponentFactory(const std::string& name, Args && ... args) {
			componentFactories[name] = std::make_unique<T>();
		}

		/**
		 * @param name the name used to identify the {@link EntityBlueprint}
		 * @param blueprint the blueprint
		 */
		void addEntityBlueprint(const std::string& name, std::shared_ptr<EntityBlueprint> blueprint){
			entities[name] = blueprint;
		}

		/**
		 * Add all {@link Component}s found in a blueprint to the supplied entity.
		 *
		 * @param entity the entity to add the {@link Component}s to.
		 * @param blueprintname the name used to identify the {@link EntityBlueprint}
		 * @return true on success.
		 */
		bool assemble(Entity* entity, const std::string& blueprintname);
	};
}

#ifdef USING_ECSTASY
	using ecstasy::EntityFactory;
#endif
