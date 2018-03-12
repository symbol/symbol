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
	}
}

namespace catapult { namespace chain {

	/// Provides batch updating of an unconfirmed transactions cache.
	class UtUpdater {
	public:
		/// Creates an updater around \a transactionsCache with execution configuration (\a config),
		/// current time supplier (\a timeSupplier) and failed transaction sink (\a failedTransactionSink).
		/// \a confirmedCatapultCache is the real (confirmed) catapult cache.
		UtUpdater(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				const ExecutionConfiguration& config,
				const TimeSupplier& timeSupplier,
				const FailedTransactionSink& failedTransactionSink);

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
