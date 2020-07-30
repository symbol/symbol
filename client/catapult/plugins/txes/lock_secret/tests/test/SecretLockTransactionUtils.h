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

#pragma once
#include "plugins/txes/lock_shared/tests/test/LockTransactionUtils.h"

namespace catapult { namespace test {

	/// Creates a random secret proof transaction based on \a TTraits with proof of \a proofSize bytes.
	template<typename TTraits>
	auto CreateRandomSecretProofTransaction(uint16_t proofSize) {
		using TransactionType = typename TTraits::TransactionType;
		uint32_t entitySize = SizeOf32<TransactionType>() + proofSize;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
		pTransaction->Size = entitySize;
		pTransaction->ProofSize = proofSize;
		return pTransaction;
	}
}}
