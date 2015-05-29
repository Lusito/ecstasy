#pragma once
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
#include "../utils/SnapshotVector.h"

namespace Ashley {
	template<typename T> class Signal;
	
	/**
	 * A simple Receiver interface used to listen to a {@link Signal}.
	 * @author Stefan Bachmann
	 */
	template<typename T>
	class Receiver {
	public:
		virtual ~Receiver() {}

		/**
		 * @param signal The Signal that triggered event
		 * @param object The object passed on dispatch
		 */
		virtual void receive (Signal<T> &signal, T object) = 0;
	};
	
	/**
	 * A Signal is a basic event class then can dispatch an event to multiple Receivers. It uses generics to allow any type of object
	 * to be passed around on dispatch.
	 * @author Stefan Bachmann
	 */
	template<typename T>
	class Signal {
	private:
		SnapshotVector<Receiver<T> *> receivers;

	public:
		/**
		 * Add a receiver to this Signal
		 * @param receiver The receiver to be added
		 */
		void add(Receiver<T> *receiver) {
			receivers.add(receiver);
		}

		/**
		 * Remove a receiver from this Signal
		 * @param receiver The receiver to remove
		 */
		void remove(Receiver<T> *receiver) {
			receivers.remove(receiver);
		}

		/** Removes all Receivers attached to this {@link Signal}. */
		void removeAll() {
			receivers.removeAll();
		}

		/**
		 * Dispatches an event to all Receivers registered to this Signal
		 * @param object The object to send off
		 */
		void dispatch (T object) {
			receivers.block();
			for (Receiver<T> *receiver: receivers.getValues())
				receiver->receive(*this, object);
			receivers.unblock();
		}
	};
}