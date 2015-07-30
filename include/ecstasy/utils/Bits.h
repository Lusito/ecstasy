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
#include <string>

namespace ECS {
	/**
	 *  A bitset, without size limitation, allows comparison via bitwise operators to other bitfields.
	 */
	class Bits {
	private:
		uint64_t *data;
		int32_t dataLength;

	public:
		~Bits() {
			delete[] data;
		}
		
		/**
		 * Creates a bit set whose initial size is large enough to explicitly represent bits with indices in the range 0 through 63.
		 */
		Bits();

		/**
		 * Creates a bit set whose initial size is large enough to explicitly represent bits with indices in the range 0 through nbits-1.
		 * 
		 * @param nbits the initial size of the bit set
		 */
		explicit Bits(int32_t nbits);

		/**
		 * @param index the index of the bit
		 * @return whether the bit is set
		 */
		bool get(int32_t index) const;

		/**
		 * Returns the bit at the given index and clears it in one go.
		 * 
		 * @param index the index of the bit
		 * @return whether the bit was set before invocation
		 */
		bool getAndClear(int32_t index);

		/**
		 * Returns the bit at the given index and sets it in one go.
		 * 
		 * @param index the index of the bit
		 * @return whether the bit was set before invocation
		 */
		bool getAndSet(int32_t index);

		/// @param index the index of the bit to set
		void set(int32_t index);

		/// @param index the index of the bit to flip
		void flip(int32_t index);

		/// @return all longs as string, comma separated
		std::string getStringId() const;

	private:
		void checkCapacity(int32_t len);

		bool equals(const Bits &other) const;

	public:
		/// @param index the index of the bit to clear
		void clear(int32_t index);

		/// Clears the entire bitset
		void clear();

		/// @return the number of bits currently stored, <b>not</b> the highset set bit!
		int32_t numBits() const;

		/// @return the minimal number of words to store all the bits
		int32_t usedWords() const;

		/**
		 * Returns the "logical size" of this bitset: The index of the highest set bit in the bitset plus one.
		 * Returns zero if the bitset contains no set bits.
		 * 
		 * @return the logical size of this bitset
		 */
		int32_t length() const;

		/// @return true if this bitset contains no bits that are set to true
		bool isEmpty() const;

		/**
		 * Returns the index of the first bit that is set to true that occurs on or after the specified starting index.
		 * 
		 * @param fromIndex the index to start looking
		 * @return >= 0 if a truthy bit was found, -1 otherwise.
		 */
		int32_t nextSetBit(int32_t fromIndex);

		/**
		 * Returns the index of the first bit that is set to false that occurs on or after the specified starting index.
		 * 
		 * @param fromIndex the index to start looking
		 * @return >= 0 if a falsy bit was found, -1 otherwise.
		 */
		int32_t nextClearBit(int32_t fromIndex) const;

		/**
		 * Performs a logical <b>AND</b> of this target bit set with the argument bit set. This bit set is modified so
		 * that each bit in  it has the value true if and only if it both initially had the value true and the
		 * corresponding bit in the bit set argument also had the value true.
		 * 
		 * @param other The other instance
		 * @return *this
		 */
		Bits& operator &=(const Bits &other);

		/**
		 * Clears all of the bits in this instance whose corresponding bit is set in the other instance.
		 * 
		 * @param other The other instance
		 */
		void andNot(const Bits &other);

		/**
		 * Performs a logical <b>OR</b> of this instance with the other instance. This instance is modified so that a
		 * bit in it has the value true if and only if it either already had the value true or the corresponding bit in
		 * the other instance has the value true.
		 * 
		 * @param other a bit set
		 * @return *this
		 */
		Bits& operator |=(const Bits &other);

		/**
		 * Performs a logical <b>XOR</b> of this bit set with the bit set argument. This bit set is modified so that a
		 * bit in it has the value true if and only if one of the following statements holds:
		 * <ul>
		 * <li>The bit initially has the value true, and the corresponding bit in the argument has the value false.</li>
		 * <li>The bit initially has the value false, and the corresponding bit in the argument has the value true.</li>
		 * </ul>
		 * 
		 * @param other The other instance
		 * @return *this
		 */
		Bits& operator ^=(const Bits &other);

		/**
		 * Returns true if the other instance has any bits set to true that are also set to true in this instance.
		 * 
		 * @param other The other instance
		 * @return boolean indicating whether this bit set intersects the specified bit set
		 */
		bool intersects (const Bits &other) const;

		/**
		 * Returns true if this instance is a super set of the other instance, i.e. it has all bits set to true that are
		 *  also set to true in the other instance.
		 * 
		 * @param other The other instance
		 * @return boolean indicating whether this bit set is a super set of the specified set
		 */
		bool containsAll(const Bits &other) const;
		
		/**
		 * @param other The other instance
		 * @return true if all truthy bits equal the truthy bits in the other instance
		 */
		bool operator ==(const Bits &other) const {
			return equals(other);
		}
		
		/**
		 * @param other The other instance
		 * @return true if at least one truthy bit is unequal a truthy bit in the other instance
		 */
		bool operator !=(const Bits &other) const {
			return !equals(other);
		}
	};
}