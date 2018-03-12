#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "RecentHashCache.h"
#include "TransactionConsumers.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult { namespace consumers {

	namespace {
		class BlockHashCheckConsumer {
		public:
			BlockHashCheckConsumer(const chain::TimeSupplier& timeSupplier, const HashCheckOptions& options)
					: m_recentHashCache(timeSupplier, options)
			{}

		public:
			ConsumerResult operator()(const BlockElements& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				// only check block hashes when processing single elements
				if (1 != elements.size())
					return Continue();

				return m_recentHashCache.add(elements[0].EntityHash)
						? Continue()
						: Abort(Failure_Consumer_Hash_In_Recency_Cache);
			}

		private:
			RecentHashCache m_recentHashCache;
		};
	}

	disruptor::ConstBlockConsumer CreateBlockHashCheckConsumer(const chain::TimeSupplier& timeSupplier, const HashCheckOptions& options) {
		return BlockHashCheckConsumer(timeSupplier, options);
	}

	namespace {
		class TransactionHashCheckConsumer {
		public:
			TransactionHashCheckConsumer(
					const chain::TimeSupplier& timeSupplier,
					const HashCheckOptions& options,
					const chain::KnownHashPredicate& knownHashPredicate)
					: m_recentHashCache(timeSupplier, options)
					, m_knownHashPredicate(knownHashPredicate)
			{}

		public:
			ConsumerResult operator()(TransactionElements& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				auto numSkippedElements = 0u;
				for (auto& element : elements) {
					if (!shouldSkip(element))
						continue;

					// already seen
					element.Skip = true;
					++numSkippedElements;
				}

				if (elements.size() != numSkippedElements)
					return Continue();

				CATAPULT_LOG(trace) << "all " << numSkippedElements << " transaction(s) skipped in TransactionHashCheck";
				return Abort(Failure_Consumer_Hash_In_Recency_Cache);
			}

		private:
			bool shouldSkip(const model::TransactionElement& element) {
				if (!m_recentHashCache.add(element.EntityHash))
					return true;

				return m_knownHashPredicate(element.Transaction.Deadline, element.EntityHash);
			}

		private:
			RecentHashCache m_recentHashCache;
			chain::KnownHashPredicate m_knownHashPredicate;
		};
	}

	disruptor::TransactionConsumer CreateTransactionHashCheckConsumer(
			const chain::TimeSupplier& timeSupplier,
			const HashCheckOptions& options,
			const chain::KnownHashPredicate& knownHashPredicate) {
		return TransactionHashCheckConsumer(timeSupplier, options, knownHashPredicate);
	}
}}
