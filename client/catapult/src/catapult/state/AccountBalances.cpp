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

#include "AccountBalances.h"

namespace catapult { namespace state {

	namespace {
		static constexpr bool IsZero(Amount amount) {
			return Amount(0) == amount;
		}
	}

	AccountBalances::AccountBalances() = default;

	AccountBalances::AccountBalances(const AccountBalances& accountBalances) {
		*this = accountBalances;
	}

	AccountBalances::AccountBalances(AccountBalances&& accountBalances) = default;

	AccountBalances& AccountBalances::operator=(const AccountBalances& accountBalances) {
		m_optimizedMosaicId = accountBalances.optimizedMosaicId();
		m_balances.optimize(m_optimizedMosaicId);
		for (const auto& pair : accountBalances)
			m_balances.insert(pair);

		return *this;
	}

	AccountBalances& AccountBalances::operator=(AccountBalances&& accountBalances) = default;

	size_t AccountBalances::size() const {
		return m_balances.size();
	}

	CompactMosaicMap::const_iterator AccountBalances::begin() const {
		return m_balances.begin();
	}

	CompactMosaicMap::const_iterator AccountBalances::end() const {
		return m_balances.end();
	}

	MosaicId AccountBalances::optimizedMosaicId() const {
		return m_optimizedMosaicId;
	}

	Amount AccountBalances::get(MosaicId mosaicId) const {
		auto iter = m_balances.find(mosaicId);
		return m_balances.end() == iter ? Amount(0) : iter->second;
	}

	AccountBalances& AccountBalances::credit(MosaicId mosaicId, Amount amount) {
		if (IsZero(amount))
			return *this;

		auto iter = m_balances.find(mosaicId);
		if (m_balances.end() == iter)
			m_balances.insert(std::make_pair(mosaicId, amount));
		else
			iter->second = iter->second + amount;

		return *this;
	}

	AccountBalances& AccountBalances::debit(MosaicId mosaicId, Amount amount) {
		if (IsZero(amount))
			return *this;

		auto iter = m_balances.find(mosaicId);
		auto hasZeroBalance = m_balances.end() == iter;
		if (hasZeroBalance || amount > iter->second) {
			auto currentBalance = hasZeroBalance ? Amount(0) : iter->second;
			std::ostringstream out;
			out
					<< "debit amount (" << amount << ") is greater than current balance (" << currentBalance
					<< ") for mosaic " << utils::HexFormat(mosaicId);
			CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
		}

		iter->second = iter->second - amount;
		if (IsZero(iter->second))
			m_balances.erase(mosaicId);

		return *this;
	}

	void AccountBalances::optimize(MosaicId id) {
		m_balances.optimize(id);
		m_optimizedMosaicId = id;
	}
}}
