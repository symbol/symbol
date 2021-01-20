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
#include "catapult/utils/TimeSpan.h"
#include <filesystem>
#include <string>
#include <unordered_set>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace mongo {

	/// Database configuration settings.
	struct DatabaseConfiguration {
	public:
		/// Database uri.
		std::string DatabaseUri;

		/// Database name.
		std::string DatabaseName;

		/// Maximum number of database writer threads.
		uint32_t MaxWriterThreads;

		/// Maximum number of heights to drop at once.
		uint32_t MaxDropBatchSize;

		/// Write timeout.
		utils::TimeSpan WriteTimeout;

		/// Named database plugins to enable.
		std::unordered_set<std::string> Plugins;

	private:
		DatabaseConfiguration() = default;

	public:
		/// Creates an uninitialized database configuration.
		static DatabaseConfiguration Uninitialized();

	public:
		/// Loads a database configuration from \a bag.
		static DatabaseConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a database configuration from \a resourcesPath.
		static DatabaseConfiguration LoadFromPath(const std::filesystem::path& resourcesPath);
	};
}}
