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
#include "catapult/utils/BlockSpan.h"
#include <unordered_set>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Namespace plugin configuration settings.
	struct NamespaceConfiguration {
	public:
		/// Maximum namespace and mosaic name size.
		uint8_t MaxNameSize;

		/// Maximum namespace duration.
		utils::BlockSpan MaxNamespaceDuration;

		/// Grace period during which time only the previous owner can renew an expired namespace.
		utils::BlockSpan NamespaceGracePeriodDuration;

		/// Reserved root namespaces that cannot be claimed.
		std::unordered_set<std::string> ReservedRootNamespaceNames;

		/// Public key of the namespace rental fee sink account.
		Key NamespaceRentalFeeSinkPublicKey;

		/// Root namespace rental fee per block.
		Amount RootNamespaceRentalFeePerBlock;

		/// Child namespace rental fee.
		Amount ChildNamespaceRentalFee;

		/// Maximum number of children for a root namespace.
		uint16_t MaxChildNamespaces;

		/// Maximum number of mosaics that an account can own.
		uint16_t MaxMosaicsPerAccount;

		/// Maximum mosaic duration.
		utils::BlockSpan MaxMosaicDuration;

		/// Flag indicating whether an update of an existing mosaic levy is allowed or not.
		bool IsMosaicLevyUpdateAllowed;

		/// Maximum mosaic divisibility.
		uint8_t MaxMosaicDivisibility;

		/// Maximum total divisible mosaic units (total-supply * 10 ^ divisibility).
		Amount MaxMosaicDivisibleUnits;

		/// Public key of the mosaic rental fee sink account.
		Key MosaicRentalFeeSinkPublicKey;

		/// Mosaic rental fee.
		Amount MosaicRentalFee;

	private:
		NamespaceConfiguration() = default;

	public:
		/// Creates an uninitialized namespace configuration.
		static NamespaceConfiguration Uninitialized();

		/// Loads a namespace configuration from \a bag.
		static NamespaceConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
