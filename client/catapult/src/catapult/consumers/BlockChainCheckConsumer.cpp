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
			explicit BlockChainCheckConsumer(
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
				for (const auto& element : elements) {
					// check for a valid chain link
					if (nullptr != pPreviousElement && !IsLink(*pPreviousElement, element.Block))
						return Abort(Failure_Consumer_Remote_Chain_Improper_Link);

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
