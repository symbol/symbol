/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "BalanceHistory.h"
#include <algorithm>

namespace catapult { namespace state {

	size_t BalanceHistory::size() const {
		return m_heightBalanceMap.size();
	}

	std::vector<Height> BalanceHistory::heights() const {
		return m_heightBalanceMap.heights();
	}

	Amount BalanceHistory::balance() const {
		return m_heightBalanceMap.get();
	}

	Amount BalanceHistory::balance(Height height) const {
		return m_heightBalanceMap.get(height);
	}

	bool BalanceHistory::anyAtLeast(Amount minAmount) const {
		return m_heightBalanceMap.anyOf([minAmount](auto amount) {
			return minAmount <= amount;
		});
	}

	void BalanceHistory::add(Height height, Amount balance) {
		m_heightBalanceMap.add(height, balance);
	}

	void BalanceHistory::prune(Height height) {
		m_heightBalanceMap.prune(height);
	}
}}
