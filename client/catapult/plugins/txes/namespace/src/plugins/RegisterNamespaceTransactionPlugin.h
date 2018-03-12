#pragma once
#include "catapult/plugins.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace model { class TransactionPlugin; } }

namespace catapult { namespace plugins {

	/// The namespace rental fee configuration.
	struct NamespaceRentalFeeConfiguration {
		/// The public key of the rental fee sink account.
		Key SinkPublicKey;

		/// The address of the rental fee sink account.
		Address SinkAddress;

		/// The root namespace rental fee per block.
		Amount RootFeePerBlock;

		/// The child namespace rental fee.
		Amount ChildFee;

		/// The public key of the (exempt from fees) nemesis account.
		Key NemesisPublicKey;
	};

	/// Creates a register namespace transaction plugin given the rental fee configuration (\a config).
	PLUGIN_API
	std::unique_ptr<model::TransactionPlugin> CreateRegisterNamespaceTransactionPlugin(const NamespaceRentalFeeConfiguration& config);
}}
