#pragma once
#include "catapult/utils/BlockSpan.h"
#include <unordered_set>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Namespace plugin configuration settings.
	struct NamespaceConfiguration {
	public:
		/// The maximum namespace and mosaic name size.
		uint8_t MaxNameSize;

		/// The maximum namespace duration.
		utils::BlockSpan MaxNamespaceDuration;

		/// The grace period during which time only the previous owner can renew an expired namespace.
		utils::BlockSpan NamespaceGracePeriodDuration;

		/// Reserved root namespaces that cannot be claimed.
		std::unordered_set<std::string> ReservedRootNamespaceNames;

		/// The public key of the namespace rental fee sink account.
		Key NamespaceRentalFeeSinkPublicKey;

		/// The root namespace rental fee per block.
		Amount RootNamespaceRentalFeePerBlock;

		/// The child namespace rental fee.
		Amount ChildNamespaceRentalFee;

		/// The maximum number of children for a root namespace.
		uint16_t MaxChildNamespaces;

		/// The maximum number of mosaics that an account can own.
		uint16_t MaxMosaicsPerAccount;

		/// The maximum mosaic duration.
		utils::BlockSpan MaxMosaicDuration;

		/// Flag indicating whether an update of an existing mosaic levy is allowed or not.
		bool IsMosaicLevyUpdateAllowed;

		/// The maximum mosaic divisibility.
		uint8_t MaxMosaicDivisibility;

		/// The maximum total divisible mosaic units (total-supply * 10 ^ divisibility).
		Amount MaxMosaicDivisibleUnits;

		/// The public key of the mosaic rental fee sink account.
		Key MosaicRentalFeeSinkPublicKey;

		/// The mosaic rental fee.
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
