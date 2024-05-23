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

#include "Block.h"
#include <algorithm>
#include <cstring>

namespace catapult { namespace model {

	namespace {
		uint32_t GetBlockFooterSize(EntityType type) {
			return IsImportanceBlock(type) ? 0 : PaddedBlockFooter::Footer_Size;
		}
	}

	bool IsImportanceBlock(EntityType type) {
		return Entity_Type_Block_Nemesis == type || Entity_Type_Block_Importance == type;
	}

	uint32_t GetBlockHeaderSize(EntityType type) {
		return sizeof(BlockHeader) + (IsImportanceBlock(type) ? sizeof(ImportanceBlockFooter) : sizeof(PaddedBlockFooter));
	}

	RawBuffer GetBlockHeaderDataBuffer(const BlockHeader& header) {
		return { reinterpret_cast<const uint8_t*>(&header) + VerifiableEntity::Header_Size,
				 GetBlockHeaderSize(header.Type) - VerifiableEntity::Header_Size - GetBlockFooterSize(header.Type) };
	}

	size_t GetTransactionPayloadSize(const BlockHeader& header) {
		return header.Size - GetBlockHeaderSize(header.Type);
	}

	bool IsSizeValid(const Block& block, const TransactionRegistry& registry) {
		if (block.Size < sizeof(VerifiableEntity) || block.Size < GetBlockHeaderSize(block.Type)) {
			CATAPULT_LOG(warning) << block.Type << " block failed size validation with size " << block.Size;
			return false;
		}

		auto transactions = block.Transactions(EntityContainerErrorPolicy::Suppress);
		auto areAllTransactionsValid = std::all_of(transactions.cbegin(), transactions.cend(), [&registry](const auto& transaction) {
			return IsSizeValid(transaction, registry);
		});

		if (areAllTransactionsValid && !transactions.hasError())
			return true;

		CATAPULT_LOG(warning) << "block transactions failed size validation (valid sizes? " << areAllTransactionsValid << ", errors? "
							  << transactions.hasError() << ")";
		return false;
	}
}}
