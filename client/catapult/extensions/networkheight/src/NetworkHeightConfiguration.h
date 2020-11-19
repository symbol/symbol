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
#include <filesystem>
#include <string>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace networkheight {

	/// Network height configuration settings.
	struct NetworkHeightConfiguration {
	public:
		/// Number of nodes that this node should communicate with during network height detection.
		uint8_t MaxNodes;

	private:
		NetworkHeightConfiguration() = default;

	public:
		/// Creates an uninitialized network height configuration.
		static NetworkHeightConfiguration Uninitialized();

	public:
		/// Loads a network height configuration from \a bag.
		static NetworkHeightConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a network height configuration from \a resourcesPath.
		static NetworkHeightConfiguration LoadFromPath(const std::filesystem::path& resourcesPath);
	};
}}
