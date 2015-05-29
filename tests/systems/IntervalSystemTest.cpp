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
#include "../TestBase.h"
#include <ecstasy/systems/IteratingSystem.h>

namespace IntervalSystemTest {
	const float deltaTime = 0.1f;

	class IntervalSystemSpy : public IntervalSystem<IntervalSystemSpy> {
	public:
		int numUpdates = 0;

		IntervalSystemSpy() : IntervalSystem(deltaTime * 2.0f) {}

	protected:
		void updateInterval () override {
			++numUpdates;
		}
	};

	TEST_CASE("intervalSystem") {
		Engine engine;
		IntervalSystemSpy intervalSystemSpy;

		engine.addSystem(&intervalSystemSpy);

		for (int i = 1; i <= 10; ++i) {
			engine.update(deltaTime);
			REQUIRE((i / 2) == intervalSystemSpy.numUpdates);
		}
		engine.clear();
	}
}
