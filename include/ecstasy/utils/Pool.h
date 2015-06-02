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

#include <stdint.h>
#include <limits>
#include <stdexcept>
#include <vector>

namespace ECS {
	/** Objects implementing this interface will have {@link #reset()} called when passed to {@link #free(Object)}. */
	class Poolable {
	public:
		virtual ~Poolable() {}
		/** Resets the object for reuse. Object references should be nulled and fields may be set to default values. */
		virtual void reset () = 0;
	};
	
	/** A pool of objects that can be reused to avoid allocation.
	 * @author Nathan Sweet */
	template<typename T>
	class Pool {
		static_assert(std::is_base_of<Poolable, T>::value, "T must derive from Poolable");
		/** The maximum number of objects that will be pooled. */
		const uint32_t max;
		/** The highest number of free objects. Can be reset any time. */
		uint32_t peak;

	private:
		std::vector<T *> freeObjects;

	public:
		/** Creates a pool with an initial capacity of 16 and no maximum. */
		Pool () : Pool(16, std::numeric_limits<uint32_t>::max()) {
		}

		/** Creates a pool with the specified initial capacity and no maximum. */
		explicit Pool(int initialCapacity) : Pool(initialCapacity, std::numeric_limits<uint32_t>::max()) {
		}

		/** @param max The maximum number of free objects to store in this pool. */
		explicit Pool (int initialCapacity, int max) : max(max) {
			freeObjects.reserve(initialCapacity);
		}
		virtual ~Pool() {
			clear();
		}

	protected:
		virtual T* newObject () = 0;

	public:
		/** Returns an object from this pool. The object may be new (from {@link #newObject()}) or reused (previously
		 * {@link #free(Object) freed}). */
		T* obtain () {
			if(freeObjects.empty())
				return newObject();
			T* back = freeObjects.back();
			freeObjects.pop_back();
			return back;
		}

		/** Puts the specified object in the pool, making it eligible to be returned by {@link #obtain()}. If the pool already contains
		 * {@link #max} free objects, the specified object is reset but not added to the pool. */
		void free (T *object) {
			if (object == nullptr) throw std::invalid_argument("object cannot be null.");
			if (freeObjects.size() < max) {
				object->reset();
				freeObjects.push_back(object);
				peak = std::max(peak, (uint32_t)freeObjects.size());
			} else {
				delete object;
			}
		}

		/** Deletes all free objects from this pool. */
		void clear () {
			while (!freeObjects.empty()) {
				delete freeObjects.back();
				freeObjects.pop_back();
			}
		}

		/** The number of objects available to be obtained. */
		int getFree () const {
			return freeObjects.size();
		}
	};
	
	template <typename T>
	class ReflectionPool : public Pool<T> {
	public:
		/** Creates a pool with an initial capacity of 16 and no maximum. */
		ReflectionPool () : Pool<T>() {}

		/** Creates a pool with the specified initial capacity and no maximum. */
		ReflectionPool (int initialCapacity) : Pool<T>(initialCapacity) {}

		/** @param max The maximum number of free objects to store in this pool. */
		ReflectionPool (int initialCapacity, int max) : Pool<T>(initialCapacity, max) {}

	protected:
		T* newObject () override {
			return new T();
		}
	};
}
