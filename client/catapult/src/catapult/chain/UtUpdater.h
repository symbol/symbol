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
#include "ChainFunctions.h"
#include "ExecutionConfiguration.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/utils/ArraySet.h"

namespace catapult {
	namespace cache {
		class CatapultCache;
		class UtCache;
		class UtCacheModifierProxy;
	}
}

namespace catapult { namespace chain {

	/// Provides batch updating of an unconfirmed transactions cache.
	class UtUpdater {
	public:
		/// Sources of transactions that can be updated.
		enum class TransactionSource {
			/// A new transaction that is added for the first time.
			New,

			/// A previously committed transaction that was reverted.
			Reverted,

			/// An existing transaction that is reapplied.
			Existing
		};

		/// Contextual information passed to throttle.
		struct ThrottleContext {
			/// Transaction source.
			UtUpdater::TransactionSource TransactionSource;

			/// Cache height.
			Height CacheHeight;

			/// Unconfirmed catapult cache.
			const cache::ReadOnlyCatapultCache& UnconfirmedCatapultCache;

			/// Unconfirmed transactions cache.
			const cache::UtCacheModifierProxy& TransactionsCache;
		};

		/// Function signature for throttling cache additions.
		using Throttle = predicate<const model::TransactionInfo&, const ThrottleContext&>;

	public:
		/// Creates an updater around \a transactionsCache with execution configuration (\a executionConfig),
		/// current time supplier (\a timeSupplier) and failed transaction sink (\a failedTransactionSink).
		/// \a confirmedCatapultCache is the real (confirmed) catapult cache.
		/// \a throttle allows throttling (rejection) of transactions.
		/// \a minFeeMultiplier is the minimum fee multiplier of transactions allowed in the cache.
		UtUpdater(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				BlockFeeMultiplier minFeeMultiplier,
				const ExecutionConfiguration& executionConfig,
				const TimeSupplier& timeSupplier,
				const FailedTransactionSink& failedTransactionSink,
				const Throttle& throttle);

		/// Destroys the updater.
		~UtUpdater();

	public:
		/// Updates this cache by applying new transaction infos in \a utInfos.
		void update(const std::vector<model::TransactionInfo>& utInfos);

		/// Updates this cache by applying new transaction infos in \a utInfos and
		/// removing transactions with hashes in \a confirmedTransactionHashes.
		void update(const utils::HashPointerSet& confirmedTransactionHashes, const std::vector<model::TransactionInfo>& utInfos);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
