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
#include "catapult/plugins.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace model { class TransactionPlugin; } }

namespace catapult { namespace plugins {

	/// Mosaic rental fee configuration.
	struct MosaicRentalFeeConfiguration {
		/// Currency mosaic id.
		UnresolvedMosaicId CurrencyMosaicId;

		/// Address of the rental fee sink account.
		UnresolvedAddress SinkAddress;

		/// Mosaic rental fee.
		Amount Fee;

		/// Public key of the (exempt from fees) nemesis account.
		Key NemesisSignerPublicKey;
	};

	/// Creates a mosaic definition transaction plugin given the rental fee configuration (\a config).
	PLUGIN_API
	std::unique_ptr<model::TransactionPlugin> CreateMosaicDefinitionTransactionPlugin(const MosaicRentalFeeConfiguration& config);
}}
