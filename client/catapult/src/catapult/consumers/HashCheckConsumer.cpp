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
						: Abort(Neutral_Consumer_Hash_In_Recency_Cache);
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
					element.ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
					++numSkippedElements;
				}

				if (elements.size() != numSkippedElements)
					return Continue();

				CATAPULT_LOG(trace) << "all " << numSkippedElements << " transaction(s) skipped in TransactionHashCheck";
				return Abort(Neutral_Consumer_Hash_In_Recency_Cache);
			}

		private:
			bool shouldSkip(const model::TransactionElement& element) {
				if (!m_recentHashCache.add(element.MerkleComponentHash))
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
