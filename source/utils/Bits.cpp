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
#include <ecstasy/utils/Bits.h>

#include <memory.h>
#include <algorithm>
#include <sstream>

namespace ECS {
	Bits::Bits() {
		dataLength = 1;
		data = new uint64_t[dataLength];
		clear();
	}

	Bits::Bits(int32_t nbits) {
		dataLength = 1 + (nbits >> 6);
		data = new uint64_t[dataLength];
		clear();
	}

	bool Bits::get(int32_t index) const {
		int32_t word = index >> 6;
		if (word >= dataLength) return false;
		return (data[word] & (1ull << (index & 0x3F))) != 0ull;
	}

	bool Bits::getAndClear(int32_t index) {
		int32_t word = index >> 6;
		if (word >= dataLength) return false;
		uint64_t oldData = data[word];
		data[word] &= ~(1ull << (index & 0x3F));
		return data[word] != oldData;
	}

	bool Bits::getAndSet(int32_t index) {
		int32_t word = index >> 6;
		checkCapacity(word);
		uint64_t oldData = data[word];
		data[word] |= 1ull << (index & 0x3F);
		return data[word] == oldData;
	}

	void Bits::set(int32_t index) {
		int32_t word = index >> 6;
		checkCapacity(word);
		data[word] |= 1ull << (index & 0x3F);
	}

	void Bits::flip(int32_t index) {
		int32_t word = index >> 6;
		checkCapacity(word);
		data[word] ^= 1ull << (index & 0x3F);
	}

	std::string Bits::getStringId() const {
		std::ostringstream ss;

		int32_t length = usedWords();
		for (int32_t i = 0; i < length; i++) {
			ss << data[i];
			if (i + 1 != length)
				ss << ",";
		}
		return ss.str();
	}

	void Bits::checkCapacity(int32_t len) {
		if (len >= dataLength) {
			len++;
			uint64_t *newData = new uint64_t[len];
			memcpy(newData, data, dataLength*sizeof(uint64_t));
			memset(newData + dataLength, 0, (len - dataLength)*sizeof(uint64_t));
			delete[] data;
			data = newData;
			dataLength = len;
		}
	}

	void Bits::clear(int32_t index) {
		int32_t word = index >> 6;
		if (word >= dataLength) return;
		data[word] &= ~(1ull << (index & 0x3F));
	}

	void Bits::clear() {
		memset(data, 0, dataLength * sizeof(uint64_t));
	}

	int32_t Bits::numBits() const {
		return dataLength << 6;
	}

	int32_t Bits::usedWords() const {
		for (int32_t word = dataLength - 1; word >= 0; --word) {
			uint64_t dataAtWord = data[word];
			if (dataAtWord != 0)
				return word + 1;
		}
		return 0;
	}

	int32_t Bits::length() const {
		for (int32_t word = dataLength - 1; word >= 0; --word) {
			uint64_t dataAtWord = data[word];
			if (dataAtWord != 0) {
				for (int32_t bit = 63; bit >= 0; --bit) {
					if ((dataAtWord & (1ull << (bit & 0x3F))) != 0ull) {
						return (word << 6) + bit;
					}
				}
			}
		}
		return 0;
	}

	bool Bits::isEmpty() const {
		int32_t length = dataLength;
		for (int32_t i = 0; i < length; i++) {
			if (data[i] != 0ull) {
				return false;
			}
		}
		return true;
	}

	int32_t Bits::nextSetBit(int32_t fromIndex) {
		int32_t word = fromIndex >> 6;
		if (word >= dataLength) return -1;
		uint64_t dataAtWord = data[word];
		if (dataAtWord != 0) {
			for (int32_t i = fromIndex & 0x3f; i < 64; i++) {
				if ((dataAtWord & (1ull << (i & 0x3F))) != 0ull) {
					return (word << 6) + i;
				}
			}
		}
		for (word++; word < dataLength; word++) {
			if (word != 0) {
				dataAtWord = data[word];
				if (dataAtWord != 0) {
					for (int32_t i = 0; i < 64; i++) {
						if ((dataAtWord & (1ull << (i & 0x3F))) != 0ull) {
							return (word << 6) + i;
						}
					}
				}
			}
		}
		return -1;
	}

	int32_t Bits::nextClearBit(int32_t fromIndex) const{
		int32_t word = fromIndex >> 6;
		if (word >= dataLength) return -1;
		uint64_t dataAtWord = data[word];
		for (int32_t i = fromIndex & 0x3f; i < 64; i++) {
			if ((dataAtWord & (1ull << (i & 0x3F))) == 0ull) {
				return (word << 6) + i;
			}
		}
		for (word++; word < dataLength; word++) {
			if (word == 0) {
				return word << 6;
			}
			dataAtWord = data[word];
			for (int32_t i = 0; i < 64; i++) {
				if ((dataAtWord & (1ull << (i & 0x3F))) == 0ull) {
					return (word << 6) + i;
				}
			}
		}
		return -1;
	}

	Bits& Bits::operator &=(const Bits &other) {
		int32_t commonWords = std::min(dataLength, other.dataLength);
		for (int32_t i = 0; commonWords > i; i++) {
			data[i] &= other.data[i];
		}

		if (dataLength > commonWords) {
			for (int32_t i = commonWords, s = dataLength; s > i; i++) {
				data[i] = 0ull;
			}
		}
		return *this;
	}

	void Bits::andNot(const Bits &other) {
		for (int32_t i = 0, j = dataLength, k = other.dataLength; i < j && i < k; i++) {
			data[i] &= ~other.data[i];
		}
	}

	Bits& Bits::operator |=(const Bits &other) {
		int32_t commonWords = std::min(dataLength, other.dataLength);
		for (int32_t i = 0; commonWords > i; i++) {
			data[i] |= other.data[i];
		}

		if (commonWords < other.dataLength) {
			checkCapacity(other.dataLength);
			for (int32_t i = commonWords, s = other.dataLength; s > i; i++) {
				data[i] = other.data[i];
			}
		}
		return *this;
	}

	Bits& Bits::operator ^=(const Bits &other) {
		int32_t commonWords = std::min(dataLength, other.dataLength);

		for (int32_t i = 0; commonWords > i; i++) {
			data[i] ^= other.data[i];
		}

		if (dataLength > commonWords) {
			for (int32_t i = other.dataLength, s = dataLength; s > i; i++) {
				data[i] = 0ull;
			}
		} else if (commonWords < other.dataLength) {
			checkCapacity(other.dataLength);
			for (int32_t i = commonWords, s = other.dataLength; s > i; i++) {
				data[i] = other.data[i];
			}
		}
		return *this;
	}

	bool Bits::intersects(const Bits &other) const {
		for (int32_t i = std::min(dataLength, other.dataLength) - 1; i >= 0; i--) {
			if ((data[i] & other.data[i]) != 0) {
				return true;
			}
		}
		return false;
	}

	bool Bits::containsAll(const Bits &other) const {
		for (int32_t i = dataLength; i < other.dataLength; i++) {
			if (other.data[i] != 0) {
				return false;
			}
		}
		for (int32_t i = std::min(dataLength, other.dataLength) - 1; i >= 0; i--) {
			if ((data[i] & other.data[i]) != other.data[i]) {
				return false;
			}
		}
		return true;
	}

	bool Bits::equals(const Bits &other) const {
		if (this == &other)
			return true;
		int32_t commonWords = std::min(dataLength, other.dataLength);
		for (int32_t i = 0; commonWords > i; i++) {
			if (data[i] != other.data[i])
				return false;
		}

		if (dataLength == other.dataLength)
			return true;

		return length() == other.length();
	}
}
