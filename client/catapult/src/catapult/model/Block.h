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
#include "Transaction.h"
#include "TransactionContainer.h"
#include "VerifiableEntity.h"
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a block header.
	struct BlockHeader : public VerifiableEntity {
	public:
		/// Block format version.
		static constexpr int Current_Version = 3;

	public:
		/// Height of a block.
		catapult::Height Height;

		/// Timestamp of a block.
		catapult::Timestamp Timestamp;

		/// Difficulty of a block.
		catapult::Difficulty Difficulty;

		/// Fee multiplier applied to transactions contained in block.
		BlockFeeMultiplier FeeMultiplier;

		/// Hash of the previous block.
		Hash256 PreviousBlockHash;

		/// Aggregate hash of a block's transactions.
		Hash256 TransactionsHash;

		/// Aggregate hash of a block's receipts.
		Hash256 ReceiptsHash;

		/// Hash of the global chain state at this block.
		Hash256 StateHash;

		/// Public key of optional beneficiary designated by harvester.
		Key BeneficiaryPublicKey;
	};

	/// Binary layout for a block.
	struct Block : public TransactionContainer<BlockHeader, Transaction> {};

#pragma pack(pop)

	/// Gets the number of bytes containing transaction data according to \a header.
	size_t GetTransactionPayloadSize(const BlockHeader& header);

	/// Checks the real size of \a block against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const Block& block, const TransactionRegistry& registry);
}}
