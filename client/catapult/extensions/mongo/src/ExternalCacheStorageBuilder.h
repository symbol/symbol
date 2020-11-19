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
