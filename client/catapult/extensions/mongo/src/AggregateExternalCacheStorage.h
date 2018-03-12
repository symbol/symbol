#pragma once
#include "ExternalCacheStorage.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace mongo {

	/// Aggregate for loading and saving cache data to external storage.
	class AggregateExternalCacheStorage : public ExternalCacheStorage {
	public:
		/// The container of sub cache storages.
		using StorageContainer = std::vector<std::unique_ptr<ExternalCacheStorage>>;

	public:
		/// Creates an aggregate around \a storages.
		explicit AggregateExternalCacheStorage(StorageContainer&& storages)
				: ExternalCacheStorage(utils::ReduceNames(utils::ExtractNames(storages)), 0)
				, m_storages(std::move(storages))
		{}

	public:
		void saveDelta(const cache::CatapultCacheDelta& cache) override {
			for (const auto& pStorage : m_storages)
				pStorage->saveDelta(cache);
		}

		void loadAll(cache::CatapultCache& cache, Height chainHeight) const override {
			for (const auto& pStorage : m_storages)
				pStorage->loadAll(cache, chainHeight);
		}

	private:
		StorageContainer m_storages;
	};
}}
