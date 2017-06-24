#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/utils/Hashers.h"
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
			explicit BlockChainCheckConsumer(uint32_t maxChainSize) : m_maxChainSize(maxChainSize)
			{}

		public:
			ConsumerResult operator()(const BlockElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				if (elements.size() > m_maxChainSize)
					return Abort(Failure_Consumer_Remote_Chain_Too_Many_Blocks);

				utils::HashPointerSet hashes;
				const model::BlockElement* pPreviousElement = nullptr;
				for (const auto& element : elements) {
					// check for a valid chain link
					if (nullptr != pPreviousElement && !IsLink(*pPreviousElement, element.Block)) {
						CATAPULT_LOG(info) << "rejecting chain part due to improper link";
						return Abort(Failure_Consumer_Remote_Chain_Improper_Link);
					}

					// check for duplicate transactions
					for (const auto& txElement : element.Transactions) {
						if (!hashes.insert(&txElement.EntityHash).second) {
							CATAPULT_LOG(info) << "rejecting chain part due to duplicate transactions";
							return Abort(Failure_Consumer_Remote_Chain_Duplicate_Transactions);
						}
					}

					pPreviousElement = &element;
				}

				return Continue();
			}

		private:
			uint32_t m_maxChainSize;
		};
	}

	disruptor::ConstBlockConsumer CreateBlockChainCheckConsumer(uint32_t maxChainSize) {
		return BlockChainCheckConsumer(maxChainSize);
	}
}}
