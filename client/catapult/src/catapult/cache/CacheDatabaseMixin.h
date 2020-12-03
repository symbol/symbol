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
#include "CacheConfiguration.h"
#include "catapult/cache_db/CacheDatabase.h"
#include "catapult/cache_db/UpdateSet.h"
#include "catapult/deltaset/ConditionalContainer.h"

namespace catapult { namespace cache {

	/// Mixin that owns a cache database.
	class CacheDatabaseMixin {
	protected:
		/// Creates a mixin around \a config and \a columnFamilyNames with optional \a pruningMode.
		CacheDatabaseMixin(
				const CacheConfiguration& config,
				const std::vector<std::string>& columnFamilyNames,
				FilterPruningMode pruningMode = FilterPruningMode::Disabled)
				: m_pDatabase(config.ShouldUseCacheDatabase
						? std::make_unique<CacheDatabase>(CacheDatabaseSettings(
								config.CacheDatabaseDirectory,
								config.CacheDatabaseConfig,
								GetAdjustedColumnFamilyNames(config, columnFamilyNames),
								pruningMode))
						: std::make_unique<CacheDatabase>())
				, m_containerMode(GetContainerMode(config))
				, m_hasPatriciaTreeSupport(config.ShouldStorePatriciaTrees)
		{}

	protected:
		/// Returns \c true if patricia tree support is enabled.
		bool hasPatriciaTreeSupport() const {
			return m_hasPatriciaTreeSupport;
		}

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

		/// Flushes the database.
		void flush() {
			if (deltaset::ConditionalContainerMode::Storage == m_containerMode)
				database().flush();
		}

	private:
		static std::vector<std::string> GetAdjustedColumnFamilyNames(
				const CacheConfiguration& config,
				const std::vector<std::string>& columnFamilyNames) {
			auto adjustedColumnFamilyNames = columnFamilyNames;
			if (config.ShouldStorePatriciaTrees)
				adjustedColumnFamilyNames.push_back("patricia_tree");

			return adjustedColumnFamilyNames;
		}

	private:
		std::unique_ptr<CacheDatabase> m_pDatabase;
		const deltaset::ConditionalContainerMode m_containerMode;
		const bool m_hasPatriciaTreeSupport;
	};
}}
