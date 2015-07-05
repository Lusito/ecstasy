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
#include <stdarg.h>

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

	TEST_CASE("Add listener and emit") {
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

	TEST_CASE("Add listeners and emit") {
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

	TEST_CASE("Add listener, emit and disconnect") {
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

	TEST_CASE("Add listener during emit") {
		Dummy dummy;
		Signal<void(Dummy *)> signal;
		ListenerMock listenerB;

		int count = 0;
		int countB = 0;

		ConnectionRef ref = signal.connect([&](Dummy *object) {
			++count;
			//fixme: ideally this connection would not be called during the same emit.
			signal.connect([&](Dummy *object) {
				++countB;
			});
		});
		signal.connect(&listenerB, &ListenerMock::callback);

		signal.emit(&dummy);

		REQUIRE(1 == count);
		REQUIRE(1 == listenerB.count);
		REQUIRE(1 == countB);
	}

	TEST_CASE("Disconnect during emit") {
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

	TEST_CASE("Connection scope") {
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

	// Below are the original tests from Tim Janik (modified for catch).
	static const char *string_printf(const char *format, ...) {
		static char buffer[100];
		va_list args;
		va_start (args, format);
		bool success = vsprintf(buffer, format, args) >= 0;
		va_end (args);
		return success ? buffer : "";
	}
	
	struct Foo {
		char foo_bool(std::string &result, float f, int i, std::string s) {
			result += string_printf("Foo: %.2f\n", f + i + s.size());
			return true;
		}
	};
	static char float_callback(std::string &result, float f, int, std::string) {
		result += string_printf("float: %.2f\n", f);
		return 0;
	}

	TEST_CASE("Basic signal test") {
		std::string accu = "";
		Signal11::Signal<char(std::string &result, float, int, std::string)> sig1;
		auto id1 = sig1.connect(float_callback);
		auto id2 = sig1.connect([](std::string &result, float, int i, std::string) { result += string_printf("int: %d\n", i); return 0; });
		auto id3 = sig1.connect([](std::string &result, float, int, const std::string &s) { result += string_printf("string: %s\n", s.c_str()); return 0; });
		sig1.emit(accu, .3f, 4, "huhu");
		REQUIRE(id1.disconnect());
		REQUIRE(!id1.disconnect());
		REQUIRE(id2.disconnect());
		REQUIRE(id3.disconnect());
		REQUIRE(!id3.disconnect());
		REQUIRE(!id2.disconnect());
		Foo foo;
		sig1.connect(foo, &Foo::foo_bool);
		sig1.connect(&foo, &Foo::foo_bool);
		sig1.emit(accu, .5, 1, "12");

		Signal11::Signal<void(std::string, int)> sig2;
		sig2.connect([&accu](std::string msg, int) { accu += string_printf("msg: %s", msg.c_str()); });
		sig2.connect([&accu](std::string, int d)   { accu += string_printf(" *%d*\n", d); });
		sig2.emit("in sig2", 17);

		accu += "DONE";

		const char *expected =
			"float: 0.30\n"
			"int: 4\n"
			"string: huhu\n"
			"Foo: 3.50\n"
			"Foo: 3.50\n"
			"msg: in sig2 *17*\n"
			"DONE";
		REQUIRE(accu == expected);
	}


	struct TestCollectorVector {
		static int handler1()  { return 1; }
		static int handler42()  { return 42; }
		static int handler777()  { return 777; }
	};

	TEST_CASE("Return the result of the all signal handlers from a signal emission in a std::vector") {
		Signal11::Signal<int(), Signal11::CollectorVector<int>> sig_vector;
		sig_vector.connect(TestCollectorVector::handler777);
		sig_vector.connect(TestCollectorVector::handler42);
		sig_vector.connect(TestCollectorVector::handler1);
		sig_vector.connect(TestCollectorVector::handler42);
		sig_vector.connect(TestCollectorVector::handler777);
		std::vector<int> results = sig_vector.emit();
		const std::vector<int> reference = { 777, 42, 1, 42, 777, };
		REQUIRE(results == reference);
	}

	struct TestCollector {
		bool check1 = false, check2 = false;
		bool handler_true()  { check1 = true; return true; }
		bool handler_false()  { check2 = true; return false; }
		bool handler_fail()  { FAIL("Abort"); return false; }
	};

	TEST_CASE("Keep signal emissions going while all handlers return !0 (true)") {
		TestCollector self;
		Signal11::Signal<bool(), Signal11::CollectorUntil0<bool>> sig;
		sig.connect(self, &TestCollector::handler_true);
		sig.connect(self, &TestCollector::handler_false);
		sig.connect(self, &TestCollector::handler_fail);
		REQUIRE(!self.check1);
		REQUIRE(!self.check2);
		REQUIRE(!sig.emit());
		REQUIRE(self.check1);
		REQUIRE(self.check2);
	}

	TEST_CASE("Keep signal emissions going while all handlers return 0 (false)") {
		TestCollector self;
		Signal11::Signal<bool(), Signal11::CollectorWhile0<bool>> sig;
		sig.connect(self, &TestCollector::handler_false);
		sig.connect(self, &TestCollector::handler_true);
		sig.connect(self, &TestCollector::handler_fail);
		REQUIRE(!self.check1);
		REQUIRE(!self.check2);
		REQUIRE(sig.emit());
		REQUIRE(self.check1);
		REQUIRE(self.check2);
	}
}