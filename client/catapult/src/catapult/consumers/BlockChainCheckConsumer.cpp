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

#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/TimeSpan.h"
#include <unordered_set>

namespace catapult { namespace consumers {

	namespace {
		namespace {
			bool IsLink(const model::BlockElement& previousElement, const model::Block& currentBlock) {
				return chain::IsChainLink(previousElement.Block, previousElement.EntityHash, currentBlock);
			}
		}

		class BlockChainCheckConsumer {
		public:
			BlockChainCheckConsumer(
					uint32_t maxChainSize,
					const utils::TimeSpan& maxBlockFutureTime,
					const chain::TimeSupplier& timeSupplier)
					: m_maxChainSize(maxChainSize)
					, m_maxBlockFutureTime(maxBlockFutureTime)
					, m_timeSupplier(timeSupplier)
			{}

		public:
			ConsumerResult operator()(const BlockElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				if (elements.size() > m_maxChainSize)
					return Abort(Failure_Consumer_Remote_Chain_Too_Many_Blocks);

				if (!isChainTimestampAllowed(elements.back().Block.Timestamp))
					return Abort(Failure_Consumer_Remote_Chain_Too_Far_In_Future);

				utils::HashPointerSet hashes;
				const model::BlockElement* pPreviousElement = nullptr;
				const Hash256* pPreviousImportanceBlockHash = nullptr;
				for (const auto& element : elements) {
					// check for a valid chain link
					if (pPreviousElement && !IsLink(*pPreviousElement, element.Block))
						return Abort(Failure_Consumer_Remote_Chain_Improper_Link);

					// check for importance link
					if (model::IsImportanceBlock(element.Block.Type, element.Block.Version)) {
						const auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(element.Block);
						if (pPreviousImportanceBlockHash && *pPreviousImportanceBlockHash != blockFooter.PreviousImportanceBlockHash) {
							CATAPULT_LOG(warning)
									<< "block at height " << element.Block.Height << " has PreviousImportanceBlockHash "
									<< blockFooter.PreviousImportanceBlockHash << " but " << *pPreviousImportanceBlockHash
									<< " is expected";
							return Abort(Failure_Consumer_Remote_Chain_Improper_Importance_Link);
						}

						pPreviousImportanceBlockHash = &element.EntityHash;
					}

					// check for duplicate transactions
					for (const auto& transactionElement : element.Transactions) {
						if (!hashes.insert(&transactionElement.EntityHash).second)
							return Abort(Failure_Consumer_Remote_Chain_Duplicate_Transactions);
					}

					pPreviousElement = &element;
				}

				return Continue();
			}

		private:
			bool isChainTimestampAllowed(Timestamp chainTimestamp) const {
				return chainTimestamp <= m_timeSupplier() + m_maxBlockFutureTime;
			}

		private:
			uint32_t m_maxChainSize;
			utils::TimeSpan m_maxBlockFutureTime;
			chain::TimeSupplier m_timeSupplier;
		};
	}

	disruptor::ConstBlockConsumer CreateBlockChainCheckConsumer(
			uint32_t maxChainSize,
			const utils::TimeSpan& maxBlockFutureTime,
			const chain::TimeSupplier& timeSupplier) {
		return BlockChainCheckConsumer(maxChainSize, maxBlockFutureTime, timeSupplier);
	}
}}
