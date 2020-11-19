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
#include "catapult/types.h"
#include <filesystem>
#include <string>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace timesync {

	/// Time synchronization configuration settings.
	struct TimeSynchronizationConfiguration {
	public:
		/// Number of nodes that this node should communicate with during time synchronization.
		uint8_t MaxNodes;

		/// Minimum importance a node must have in order to be considered as a synchronization partner.
		Importance MinImportance;

	private:
		TimeSynchronizationConfiguration() = default;

	public:
		/// Creates an uninitialized time synchronization configuration.
		static TimeSynchronizationConfiguration Uninitialized();

	public:
		/// Loads a time synchronization configuration from \a bag.
		static TimeSynchronizationConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a time synchronization configuration from \a resourcesPath.
		static TimeSynchronizationConfiguration LoadFromPath(const std::filesystem::path& resourcesPath);
	};
}}
