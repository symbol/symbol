#pragma once
#include <string>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// The user configuration settings.
	struct UserConfiguration {
	public:
		/// The boot key.
		std::string BootKey;

		/// The harvest key.
		std::string HarvestKey;

		/// \c true if auto harvesting is enabled.
		bool IsAutoHarvestingEnabled;

		/// The maximum number of unlocked accounts.
		uint32_t MaxUnlockedAccounts;

		/// The data directory.
		std::string DataDirectory;

		/// The plugins directory.
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
