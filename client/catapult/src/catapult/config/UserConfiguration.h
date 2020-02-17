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
#include <string>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// User configuration settings.
	struct UserConfiguration {
	public:
		/// Boot private key.
		std::string BootPrivateKey;

		/// \c true if potential delegated harvesters should be automatically detected.
		bool EnableDelegatedHarvestersAutoDetection;

		/// Data directory.
		std::string DataDirectory;

		/// Certificate directory.
		std::string CertificateDirectory;

		/// Plugins directory.
		std::string PluginsDirectory;

	private:
		UserConfiguration() = default;

	public:
		/// Creates an uninitialized user configuration.
		static UserConfiguration Uninitialized();

	public:
		/// Loads a user configuration from \a bag.
		static UserConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
