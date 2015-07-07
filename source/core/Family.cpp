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
#include <ecstasy/core/Family.h>
#include <ecstasy/core/Entity.h>
#include <memory>
#include <sstream>

namespace ECS {
	static std::map<std::string, std::shared_ptr<Family>> families;

	static void addBitsString(std::ostringstream &ss, const std::string &prefix, Bits *bits) {
		ss << prefix << bits->getStringId() << ";";
	}

	static std::string getFamilyHash(Bits *all, Bits *one, Bits *exclude) {
		std::ostringstream ss;
		if (!all->isEmpty())
			addBitsString(ss, "a:", all);
		if (!one->isEmpty())
			addBitsString(ss, "o:", one);
		if (!exclude->isEmpty())
			addBitsString(ss, "e:", exclude);
		return ss.str();
	}
	
	FamilyBuilder::~FamilyBuilder() {
		delete m_all;
		delete m_one;
		delete m_exclude;
	}
	
	FamilyBuilder &FamilyBuilder::reset () {
		m_all->clear();
		m_one->clear();
		m_exclude->clear();
		return *this;
	}

	const Family &FamilyBuilder::get () {
		auto hash = getFamilyHash(m_all, m_one, m_exclude);
		auto it = families.find(hash);
		if(it != families.end())
			return *it->second.get();
		auto *family = new Family(m_all, m_one, m_exclude);
		families.emplace(hash, std::shared_ptr<Family>(family));
		m_all = new Bits();
		m_one = new Bits();
		m_exclude = new Bits();
		return *family;
	}

	FamilyBuilder Family::builder;

	Family::~Family() {
		delete m_all;
		delete m_one;
		delete m_exclude;
	}

	bool Family::matches(Entity *entity) const {
		auto &entityComponentBits = entity->getComponentBits();

		if (!entityComponentBits.containsAll(*m_all))
			return false;

		if (!m_one->isEmpty() && !m_one->intersects(entityComponentBits))
			return false;

		if (!m_exclude->isEmpty() && m_exclude->intersects(entityComponentBits))
			return false;

		return true;
	}
}
