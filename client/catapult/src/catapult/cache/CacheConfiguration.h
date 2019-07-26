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
#include "catapult/utils/FileSize.h"
#include <string>

namespace catapult { namespace cache {

	/// Possible patricia tree storage modes.
	enum class PatriciaTreeStorageMode {
		/// Patricia tree storage should be disabled.
		Disabled,

		/// Patricia tree storage should be enabled.
		Enabled
	};

	/// Cache configuration.
	struct CacheConfiguration {
	public:
		/// Creates a default cache configuration.
		CacheConfiguration()
				: ShouldUseCacheDatabase(false)
				, ShouldStorePatriciaTrees(false)
		{}

		/// Creates a cache configuration around \a databaseDirectory, \a maxCacheDatabaseWriteBatchSize
		/// and specified patricia tree storage \a mode.
		CacheConfiguration(
				const std::string& databaseDirectory,
				utils::FileSize maxCacheDatabaseWriteBatchSize,
				PatriciaTreeStorageMode mode)
				: ShouldUseCacheDatabase(true)
				, CacheDatabaseDirectory(databaseDirectory)
				, MaxCacheDatabaseWriteBatchSize(maxCacheDatabaseWriteBatchSize)
				, ShouldStorePatriciaTrees(PatriciaTreeStorageMode::Enabled == mode)
		{}

	public:
		/// \c true if a cache database should be used, \c false otherwise.
		bool ShouldUseCacheDatabase;

		/// Base directory to use for storing cache database.
		std::string CacheDatabaseDirectory;

		/// Maximum size of database write batch.
		utils::FileSize MaxCacheDatabaseWriteBatchSize;

		/// \c true if patricia trees should be stored, \c false otherwise.
		bool ShouldStorePatriciaTrees;
	};
}}
