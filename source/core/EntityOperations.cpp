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
#include <ecstasy/core/EntityOperations.h>
#include <ecstasy/core/Entity.h>
#include <ecstasy/core/Engine.h>
#include <ecstasy/core/Component.h>

namespace ECS {

	bool EntityOperationHandler::isActive() {
		return engine.updating;
	}

	void EntityOperationHandler::onAdd(EntityOperation *operation) {
		engine.addEntityInternal(operation->entity);
	}
	
	void EntityOperationHandler::onRemove(EntityOperation *operation) {
		engine.removeEntityInternal(operation->entity);
	}
	
	void EntityOperationHandler::onRemoveAll(EntityOperation *operation) {
		while(!engine.entities.empty()) {
			engine.removeEntityInternal(engine.entities.back());
		}
	}

	bool ComponentOperationHandler::isActive() {
		return engine.updating;
	}
	
	void ComponentOperationHandler::onAdd(ComponentOperation *operation) {
		operation->entity->addInternal(operation->component);
	}
	
	void ComponentOperationHandler::onRemove(ComponentOperation *operation) {
		operation->entity->removeInternal(operation->componentType);
	}
	
	void ComponentOperationHandler::onRemoveAll(ComponentOperation *operation) {
		operation->entity->removeAllInternal();
	}
}
