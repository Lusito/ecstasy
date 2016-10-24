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
#include "../TestBase.hpp"
#include <ecstasy/systems/IntervalIteratingSystem.hpp>

namespace IntervalIteratingSystemTests {
	const float deltaTime = 0.1f;

	struct IntervalComponentSpy: public Component<IntervalComponentSpy> {
		int numUpdates = 0;
	};

	class IntervalIteratingSystemSpy : public IntervalIteratingSystem<IntervalIteratingSystemSpy> {
	public:
		IntervalIteratingSystemSpy ()
			: IntervalIteratingSystem(Family::all<IntervalComponentSpy>().get(), deltaTime * 2.0f) {
		}

	protected:
		void processEntity (Entity* entity) override {
			entity->get<IntervalComponentSpy>()->numUpdates++;
		}
	};

	TEST_CASE("intervalIteratingSystem") {
		Engine engine;
		auto entities = engine.getEntitiesFor(Family::all<IntervalComponentSpy>().get());

		engine.emplaceSystem<IntervalIteratingSystemSpy>();

		for (int i = 0; i < 10; ++i) {
			auto entity = engine.createEntity();
			entity->emplace<IntervalComponentSpy>();
			engine.addEntity(entity);
		}

		for (int i = 1; i <= 10; ++i) {
			engine.update(deltaTime);

			for (auto e : *entities) {
				REQUIRE((i / 2) == e->get<IntervalComponentSpy>()->numUpdates);
			}
		}
	}
}
