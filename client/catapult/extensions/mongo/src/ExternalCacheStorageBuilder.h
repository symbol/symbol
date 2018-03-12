#pragma once
#include "AggregateExternalCacheStorage.h"
#include "catapult/exceptions.h"

namespace catapult { namespace mongo {

	/// Builder for creating an aggregate external cache storage around external cache storages.
	class ExternalCacheStorageBuilder {
	public:
		/// Adds \a pStorage to the builder.
		template<typename TStorage>
		void add(std::unique_ptr<TStorage>&& pStorage) {
			auto id = pStorage->id();
			m_storages.resize(std::max(m_storages.size(), id + 1));
			if (m_storages[id])
				CATAPULT_THROW_INVALID_ARGUMENT_1("storage has already been registered with id", id);

			m_storages[id] = std::move(pStorage);
		}

		/// Builds an aggregate external cache storage.
		std::unique_ptr<ExternalCacheStorage> build() {
			std::vector<std::unique_ptr<ExternalCacheStorage>> storages;
			for (auto& pStorage : m_storages) {
				if (!!pStorage)
					storages.push_back(std::move(pStorage));
			}

			CATAPULT_LOG(debug) << "creating ExternalCacheStorage with " << storages.size() << " storages";
			return std::make_unique<AggregateExternalCacheStorage>(std::move(storages));
		}

	private:
		std::vector<std::unique_ptr<ExternalCacheStorage>> m_storages;
	};
}}
