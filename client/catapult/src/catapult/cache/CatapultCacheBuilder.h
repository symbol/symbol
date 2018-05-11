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
#include "CatapultCache.h"
#include "SubCachePluginAdapter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	/// Builder for creating a catapult cache around subcaches.
	class CatapultCacheBuilder {
	public:
		/// Adds \a pSubCache to the builder with the specified storage traits.
		template<typename TStorageTraits, typename TCache>
		void add(std::unique_ptr<TCache>&& pSubCache) {
			auto id = static_cast<size_t>(TCache::Id);
			m_subCaches.resize(std::max(m_subCaches.size(), id + 1));
			if (m_subCaches[id])
				CATAPULT_THROW_INVALID_ARGUMENT_1("subcache has already been registered with id", id);

			m_subCaches[id] = std::make_unique<cache::SubCachePluginAdapter<TCache, TStorageTraits>>(std::move(pSubCache));
		}

		/// Builds a catapult cache.
		CatapultCache build() {
			CATAPULT_LOG(debug) << "creating CatapultCache with " << m_subCaches.size() << " subcaches";
			return CatapultCache(std::move(m_subCaches));
		}

	private:
		std::vector<std::unique_ptr<SubCachePlugin>> m_subCaches;
	};
}}
