#pragma once
#include "ExecutionConfiguration.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/utils/HashSet.h"

namespace catapult {
	namespace cache {
		class CatapultCache;
		class UtCache;
	}
}

namespace catapult { namespace chain {

	/// Provides batch updating of an unconfirmed transactions cache.
	class UnconfirmedTransactionsUpdater {
	private:
		using TimeProvider = std::function<Timestamp ()>;

	public:
		/// Creates an updater around \a transactionsCache with execution configuration (\a config) and
		/// current time provider (\a timeProvider).
		/// \a confirmedCatapultCache is the real (confirmed) catapult cache.
		UnconfirmedTransactionsUpdater(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				const ExecutionConfiguration& config,
				const TimeProvider& timeProvider);

		/// Destroys the updater.
		~UnconfirmedTransactionsUpdater();

	public:
		/// Updates this cache by applying new transaction infos in \a unconfirmedTransactionInfos.
		void update(std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos);

		/// Updates this cache by applying new transaction infos in \a unconfirmedTransactionInfos and
		/// removing transactions with hashes in \a confirmedTransactionHashes.
		void update(
				const utils::HashPointerSet& confirmedTransactionHashes,
				std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
