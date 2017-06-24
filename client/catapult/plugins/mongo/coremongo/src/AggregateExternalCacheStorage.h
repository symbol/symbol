#pragma once
#include "ExternalCacheStorage.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace mongo { namespace plugins {

	/// Aggregate for loading and saving cache data to external storage.
	class AggregateExternalCacheStorage : public ExternalCacheStorage {
	public:
		/// The container of sub cache storages.
		using StorageContainer = std::vector<std::unique_ptr<ExternalCacheStorage>>;

	public:
		/// Creates an aggregate around \a storages.
		explicit AggregateExternalCacheStorage(StorageContainer&& storages)
				: m_storages(std::move(storages))
				, m_name(utils::ReduceNames(utils::ExtractNames(m_storages)))
		{}

	public:
		const std::string& name() const final override {
			return m_name;
		}

		size_t id() const final override {
			return 0;
		}

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
		std::string m_name;
	};
}}}
