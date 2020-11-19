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
#include "ExternalCacheStorage.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace mongo {

	/// Aggregate for saving cache data to external storage.
	class AggregateExternalCacheStorage : public ExternalCacheStorage {
	public:
		/// Container of sub cache storages.
		using StorageContainer = std::vector<std::unique_ptr<ExternalCacheStorage>>;

	public:
		/// Creates an aggregate around \a storages.
		explicit AggregateExternalCacheStorage(StorageContainer&& storages)
				: ExternalCacheStorage(utils::ReduceNames(utils::ExtractNames(storages)), 0)
				, m_storages(std::move(storages))
		{}

	public:
		void saveDelta(const cache::CacheChanges& changes) override {
			for (const auto& pStorage : m_storages)
				pStorage->saveDelta(changes);
		}

	private:
		StorageContainer m_storages;
	};
}}
