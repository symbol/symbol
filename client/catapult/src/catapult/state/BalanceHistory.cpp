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
		auto heights = std::vector<Height>();
		for (auto iter = m_heightBalanceMap.crbegin(); m_heightBalanceMap.crend() != iter; ++iter)
			heights.push_back(iter->first);

		return heights;
	}

	Amount BalanceHistory::balance() const {
		return m_heightBalanceMap.empty() ? Amount() : (m_heightBalanceMap.cbegin())->second;
	}

	Amount BalanceHistory::balance(Height height) const {
		auto iter = m_heightBalanceMap.lower_bound(height);
		return m_heightBalanceMap.cend() == iter ? Amount() : iter->second;
	}

	bool BalanceHistory::anyAtLeast(Amount amount) const {
		return std::any_of(m_heightBalanceMap.cbegin(), m_heightBalanceMap.cend(), [amount](const auto& pair) {
			return amount <= pair.second;
		});
	}

	void BalanceHistory::add(Height height, Amount balance) {
		auto iter = m_heightBalanceMap.insert_or_assign(height, balance).first;

		// delete larger height entry with same balance
		if (m_heightBalanceMap.cbegin() != iter && (--iter)->second == balance)
			m_heightBalanceMap.erase(iter);

		// delete new height entry if smaller height entry with same balance exists
		if (this->balance(height - Height(1)) == balance)
			m_heightBalanceMap.erase(height);
	}

	void BalanceHistory::prune(Height height) {
		auto iter = m_heightBalanceMap.lower_bound(height);
		if (m_heightBalanceMap.end() == iter)
			return;

		auto balanceAtPruneHeight = iter->second;
		m_heightBalanceMap.erase(iter, m_heightBalanceMap.end());
		add(height, balanceAtPruneHeight);
	}
}}
