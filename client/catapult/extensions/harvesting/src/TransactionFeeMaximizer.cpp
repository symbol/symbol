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

#include "TransactionFeeMaximizer.h"
#include "catapult/model/FeeUtils.h"

namespace catapult { namespace harvesting {

	namespace {
		Amount CalculateTotalFee(const FeePolicy& policy) {
			return Amount(policy.FeeMultiplier.unwrap() * policy.BaseFee.unwrap());
		}
	}

	const FeePolicy& TransactionFeeMaximizer::best() const {
		return m_best;
	}

	void TransactionFeeMaximizer::apply(const model::TransactionInfo& transactionInfo) {
		auto lastFeeMultiplier = m_current.FeeMultiplier;

		const auto& transaction = *transactionInfo.pEntity;
		m_current.FeeMultiplier = model::CalculateTransactionMaxFeeMultiplier(transaction);
		m_current.BaseFee = m_current.BaseFee + model::CalculateTransactionFee(BlockFeeMultiplier(1), transaction);
		++m_current.NumTransactions;

		if (0 != m_best.NumTransactions && lastFeeMultiplier < m_current.FeeMultiplier)
			CATAPULT_THROW_INVALID_ARGUMENT("applied transactions must be sorted by fee multiplier");

		// when total fee amounts are equal, prefer more transactions
		if (CalculateTotalFee(m_best) <= CalculateTotalFee(m_current))
			m_best = m_current;
	}
}}
