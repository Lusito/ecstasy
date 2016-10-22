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

#include <ecstasy/core/Entity.hpp>
#include <map>
#include <unordered_map>
#include <string>
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

	class ComponentFactory {
	public:
		virtual bool assemble(Entity *entity, ComponentBlueprint &blueprint) = 0;
	};

	template<typename T>
	class SimpleComponentFactory : public ComponentFactory {
	public:
		bool assemble(Entity *entity, ComponentBlueprint &blueprint) override {
			return entity->assign<T>();
		};
	};
	
	class EntityFactory {
	private:
		std::unordered_map<std::string, std::unique_ptr<ComponentFactory>> componentFactories;
		std::unordered_map<std::string, std::shared_ptr<EntityBlueprint>> entities;

	public:
		template <typename T, typename ... Args>//fixme: make sure it extends ComponentFactory
		void addComponentFactory(const std::string& key, Args && ... args) {
			componentFactories[key] = std::make_unique<T>();
		}
		
		void addEntityBlueprint(const std::string& key, std::shared_ptr<EntityBlueprint> value);
		
		bool assemble(Entity *entity, const std::string& blueprintname);
	};
}

#ifdef USING_ECSTASY
	using ECS::ComponentBlueprint;
	using ECS::EntityBlueprint;
	using ECS::ComponentFactory;
	using ECS::SimpleComponentFactory;
	using ECS::EntityFactory;
#endif
