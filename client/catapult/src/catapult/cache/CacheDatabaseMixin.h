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
#include "CacheConfiguration.h"
#include "catapult/cache_db/CacheDatabase.h"
#include "catapult/deltaset/ConditionalContainer.h"

namespace catapult { namespace cache {

	/// Mixin that owns a cache database.
	class CacheDatabaseMixin {
	protected:
		/// Creates a mixin around \a config and \a columnFamilyNames.
		CacheDatabaseMixin(const CacheConfiguration& config, const std::vector<std::string>& columnFamilyNames)
				: m_pDatabase(config.ShouldUseCacheDatabase
						? std::make_unique<CacheDatabase>(config.CacheDatabaseDirectory, columnFamilyNames)
						: std::make_unique<CacheDatabase>())
		{}

	protected:
		/// Gets the database.
		CacheDatabase& database() {
			return *m_pDatabase;
		}

	protected:
		/// Gets the container mode specified by \a config.
		static deltaset::ConditionalContainerMode GetContainerMode(const CacheConfiguration& config) {
			return config.ShouldUseCacheDatabase
					? deltaset::ConditionalContainerMode::Storage
					: deltaset::ConditionalContainerMode::Memory;
		}

	private:
		std::unique_ptr<CacheDatabase> m_pDatabase;
	};
}}
