/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MosaicGlobalRestriction.h"

namespace catapult { namespace state {

	MosaicGlobalRestriction::MosaicGlobalRestriction(MosaicId mosaicId) : m_mosaicId(mosaicId)
	{}

	MosaicId MosaicGlobalRestriction::mosaicId() const {
		return m_mosaicId;
	}

	size_t MosaicGlobalRestriction::size() const {
		return m_keyRulePairs.size();
	}

	std::set<uint64_t> MosaicGlobalRestriction::keys() const {
		return m_keyRulePairs.keys();
	}

	bool MosaicGlobalRestriction::tryGet(uint64_t key, RestrictionRule& rule) const {
		return m_keyRulePairs.tryGet(key, rule);
	}

	void MosaicGlobalRestriction::set(uint64_t key, const RestrictionRule rule) {
		if (model::MosaicRestrictionType::NONE == rule.RestrictionType)
			m_keyRulePairs.remove(key);
		else
			m_keyRulePairs.set(key, rule);
	}
}}
