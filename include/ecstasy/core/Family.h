#pragma once
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

#include <map>
#include "Types.h"
#include "../utils/Bits.h"

namespace ECS {
	class Entity;
	
	ECS_UUID_TYPE(FamilyType);
	
	class Family;

/// \cond HIDDEN_SYMBOLS
	template<typename... Args>
	Bits getComponentTypeBits() {
		Bits bits;
		getComponentTypeBits<Args...>(bits);
		return bits;
	}

	template <typename T>
	void getComponentTypeBits(Bits &bits) {
		bits.set((int32_t)getComponentType<T>());
	}

	template<typename T1, typename T2, typename... Args>
	void getComponentTypeBits(Bits &bits) {
		getComponentTypeBits<T1>(bits);
		getComponentTypeBits<T2, Args...>(bits);
	}
/// \endcond

	/**
	 * A builder pattern to create Family objects.
	 * Use Family::all(), Family::one() or Family::exclude() to start
	 */
	class FamilyBuilder {
		friend class Family;
	private:
		Bits *m_all = new Bits();
		Bits *m_one = new Bits();
		Bits *m_exclude = new Bits();

		FamilyBuilder() {}

	public:
		~FamilyBuilder();

		/**
		 * Resets the builder instance
		 * 
		 * @return *this for chaining
		 */
		FamilyBuilder &reset ();

		/**
		 * @tparam Args Entities will have to contain all of the specified components.
		 * @return *this for chaining
		 */
		template<typename... Args>
		FamilyBuilder &all() {
			getComponentTypeBits<Args...>(*m_all);
			return *this;
		}

		/**
		 * @tparam Args Entities will have to contain at least one of the specified components.
		 * @return *this for chaining
		 */
		template<typename... Args>
		FamilyBuilder &one() {
			getComponentTypeBits<Args...>(*m_one);
			return *this;
		}

		/**
		 * @tparam Args Entities cannot contain any of the specified components.
		 * @return *this for chaining
		 */
		template<typename... Args>
		FamilyBuilder &exclude() {
			getComponentTypeBits<Args...>(*m_exclude);
			return *this;
		}

		/// @return A Family for the configured component types
		const Family &get ();
	};
	
	/**
	 * Represents a group of {@link Component}s. It is used to describe what Entity objects an EntitySystem should
	 * process. Families can't be instantiated directly but must be accessed via a builder.
	 * This is to avoid duplicate families that describe the same components
	 * Start with Family::all(), Family::one() or Family::exclude().
	 */
	class Family
	{
	private:
		static FamilyBuilder builder;

		Bits *m_all;
		Bits *m_one;
		Bits *m_exclude;
		
	public:
		/// The unique identifier of this Family
		const FamilyType index;

	private:
		friend class FamilyBuilder;
		// Private constructor
		Family(Bits *all, Bits *one, Bits *exclude) : m_all(all), m_one(one), m_exclude(exclude), index(getUniqueTypeId<FamilyType>()) {}

		// Do not copy
		Family(const Family &other) : index(0) {}
		
	public:
		~Family();
		
		/**
		 * @param entity An entity
		 * @return Whether the entity matches the family requirements or not
		 */
		bool matches (Entity *entity) const;

		/// @return A builder singleton instance to get a Family
		static FamilyBuilder &all() {
			return builder.reset();
		}
		
		/**
		 * @tparam Args Entities will have to contain all of the specified components.
		 * @return A builder singleton instance to get a Family
		 */
		template<typename... Args>
		static FamilyBuilder &all() {
			return builder.reset().all<Args...>();
		}

		/**
		 * @tparam Args Entities will have to contain at least one of the specified components.
		 * @return A builder singleton instance to get a Family
		 */
		template<typename... Args>
		static FamilyBuilder &one() {
			return builder.reset().one<Args...>();
		}

		/**
		 * @tparam Args Entities cannot contain any of the specified components.
		 * @return A builder singleton instance to get a Family
		 */
		template<typename... Args>
		static FamilyBuilder &exclude() {
			return builder.reset().exclude<Args...>();
		}

		/// @return true if the families are equal
		bool operator == (const Family &other) const {
			return this == &other;
		}

		/// @return true if the families are unequal
		bool operator != (const Family &other) const {
			return this != &other;
		}
	};
}

#ifdef USING_ECSTASY
	using ECS::Family;
#endif
