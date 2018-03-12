#pragma once
#include "catapult/plugins.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace model { class TransactionPlugin; } }

namespace catapult { namespace plugins {

	/// The mosaic rental fee configuration.
	struct MosaicRentalFeeConfiguration {
		/// The public key of the rental fee sink account.
		Key SinkPublicKey;

		/// The address of the rental fee sink account.
		Address SinkAddress;

		/// The mosaic rental fee.
		Amount Fee;

		/// The public key of the (exempt from fees) nemesis account.
		Key NemesisPublicKey;
	};

	/// Creates a mosaic definition transaction plugin given the rental fee configuration (\a config).
	PLUGIN_API
	std::unique_ptr<model::TransactionPlugin> CreateMosaicDefinitionTransactionPlugin(const MosaicRentalFeeConfiguration& config);
}}
