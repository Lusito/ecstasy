#pragma once
#include "Entity.h"

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

namespace ECS {
	/**
	 * Gets notified of {@link Entity} related events.
	 * @author David Saltares
	 */
	class EntityListener {
	public:
		virtual ~EntityListener() {}

		/**
		 * Called whenever an {@link Entity} is added to {@link Engine} or a specific {@link Family} See
		 * {@link Engine#addEntityListener(EntityListener)} and {@link Engine#addEntityListener(Family, EntityListener)}
		 * @param entity
		 */
		virtual void entityAdded(Entity *entity) = 0;

		/**
		 * Called whenever an {@link Entity} is removed from {@link Engine} or a specific {@link Family} See
		 * {@link Engine#addEntityListener(EntityListener)} and {@link Engine#addEntityListener(Family, EntityListener)}
		 * @param entity
		 */
		virtual void entityRemoved(Entity *entity) = 0;
	};
}