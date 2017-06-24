#pragma once
#include <stdint.h>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Multisig plugin configuration settings.
	struct MultisigConfiguration {
	public:
		/// The maximum number of multisig levels.
		uint8_t MaxMultisigDepth;

		/// The maximum number of cosigners per account.
		uint8_t MaxCosignersPerAccount;

		/// The maximum number of accounts a single account can cosign.
		uint8_t MaxCosignedAccountsPerAccount;

	private:
		MultisigConfiguration() = default;

	public:
		/// Creates an uninitialized multisig configuration.
		static MultisigConfiguration Uninitialized();

		/// Loads a multisig configuration from \a bag.
		static MultisigConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
