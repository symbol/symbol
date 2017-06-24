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
		static constexpr int Current_Version = 3;

		/// The height of a block.
		catapult::Height Height;

		/// The timestamp of a block.
		catapult::Timestamp Timestamp;

		/// The difficulty of a block.
		catapult::Difficulty Difficulty;

		/// The hash of a previous block.
		Hash256 PreviousBlockHash;

		/// The aggregate hash of a block's transactions.
		Hash256 BlockTransactionsHash;
	};

	/// Binary layout for a block.
	struct Block : public TransactionContainer<BlockHeader, Transaction> {
	};

#pragma pack(pop)

	/// Gets the number of bytes containing transaction data according to \a header.
	size_t GetTransactionPayloadSize(const BlockHeader& header);

	/// Checks the real size of \a block against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const Block& block, const TransactionRegistry& registry);
}}
