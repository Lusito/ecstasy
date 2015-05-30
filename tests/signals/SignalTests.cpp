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
#include <signal11/Signal.h>

namespace SignalTests {
	using namespace Signal11;
	class Dummy {

	};

	struct ListenerMock {
		int count = 0;

		void callback(Dummy *object) {
			++count;

			REQUIRE(object != nullptr);
		}
	};

	TEST_CASE("Add Listener and Dispatch") {
		Dummy dummy;
		Signal<void (Dummy *)> signal;
		ListenerMock listener;
		signal.connect(&listener, &ListenerMock::callback);

		for (int i = 0; i < 10; ++i) {
			REQUIRE(i == listener.count);
			signal.emit(&dummy);
			REQUIRE((i + 1) == listener.count);
		}
	}

	TEST_CASE("Add Listeners and Dispatch") {
		Dummy dummy;
		Signal<void (Dummy *)> signal;
		Allocator<ListenerMock> listeners;

		int numListeners = 10;

		for (int i = 0; i < numListeners; i++) {
			signal.connect(listeners.create(), &ListenerMock::callback);
		}

		int numDispatchs = 10;

		for (int i = 0; i < numDispatchs; ++i) {
			for (auto listener : listeners.values) {
				REQUIRE(i == listener->count);
			}

			signal.emit(&dummy);

			for (auto listener : listeners.values) {
				REQUIRE((i + 1) == listener->count);
			}
		}
	}

	TEST_CASE("Add Listener Dispatch and Remove") {
		Dummy dummy;
		Signal<void (Dummy *)> signal;
		ListenerMock listenerA;
		ListenerMock listenerB;

		signal.connect(&listenerA, &ListenerMock::callback);
		auto refB = signal.connect(&listenerB, &ListenerMock::callback);

		int numDispatchs = 5;

		for (int i = 0; i < numDispatchs; ++i) {
			REQUIRE(i == listenerA.count);
			REQUIRE(i == listenerB.count);

			signal.emit(&dummy);

			REQUIRE((i + 1) == listenerA.count);
			REQUIRE((i + 1) == listenerB.count);
		}

		refB.disconnect();

		for (int i = 0; i < numDispatchs; ++i) {
			REQUIRE((i + numDispatchs) == listenerA.count);
			REQUIRE(numDispatchs == listenerB.count);

			signal.emit(&dummy);

			REQUIRE((i + 1 + numDispatchs) == listenerA.count);
			REQUIRE(numDispatchs == listenerB.count);
		}
	}

	TEST_CASE("Remove while dispatch") {
		Dummy dummy;
		Signal<void(Dummy *)> signal;
		ListenerMock listenerB;

		int count = 0;

		ConnectionRef ref = signal.connect([&](Dummy *object) {
			++count;
			ref.disconnect();
		});
		signal.connect(&listenerB, &ListenerMock::callback);

		signal.emit(&dummy);

		REQUIRE(1 == count);
		REQUIRE(1 == listenerB.count);
	}

	TEST_CASE("Connection Scope") {
		Dummy dummy;
		Signal<void (Dummy *)> signal;

		ListenerMock listenerA;
		ListenerMock listenerB;
		{
			ConnectionScope scope;
			scope += signal.connect(&listenerA, &ListenerMock::callback);
			scope += signal.connect(&listenerB, &ListenerMock::callback);

			signal.emit(&dummy);

			REQUIRE(1 == listenerA.count);
			REQUIRE(1 == listenerB.count);
		}

		signal.emit(&dummy);

		REQUIRE(1 == listenerA.count);
		REQUIRE(1 == listenerB.count);
	}
}