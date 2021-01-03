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

#include "AccountWeights.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	AccountWeights::AccountWeights() : m_totalWeight(0)
	{}

	size_t AccountWeights::size() const {
		return m_accountWeights.size();
	}

	uint64_t AccountWeights::totalWeight() const {
		return m_totalWeight;
	}

	uint64_t AccountWeights::weight(const Key& key) const {
		auto iter = m_accountWeights.find(key);
		return m_accountWeights.cend() == iter ? 0 : iter->second;
	}

	void AccountWeights::increment(const Key& key, uint64_t delta) {
		if (0 == delta)
			return;

		m_accountWeights[key] += delta;
		m_totalWeight += delta;
	}

	void AccountWeights::decrement(const Key& key, uint64_t delta) {
		auto& weight = m_accountWeights[key];
		if (weight < delta)
			CATAPULT_THROW_RUNTIME_ERROR_1("weight cannot be decremented below zero for key", key);

		weight -= delta;
		m_totalWeight -= delta;

		if (0 == weight)
			m_accountWeights.erase(key);
	}

	void AccountWeights::reset() {
		m_accountWeights.clear();
		m_totalWeight = 0;
	}
}}
