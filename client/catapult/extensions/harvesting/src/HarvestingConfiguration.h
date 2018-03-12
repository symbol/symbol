#pragma once
#include <boost/filesystem/path.hpp>
#include <string>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace harvesting {

	/// The harvesting configuration settings.
	struct HarvestingConfiguration {
	public:
		/// The harvest key.
		std::string HarvestKey;

		/// \c true if auto harvesting is enabled.
		bool IsAutoHarvestingEnabled;

		/// The maximum number of unlocked accounts.
		uint32_t MaxUnlockedAccounts;

	private:
		HarvestingConfiguration() = default;

	public:
		/// Creates an uninitialized harvesting configuration.
		static HarvestingConfiguration Uninitialized();

	public:
		/// Loads a harvesting configuration from \a bag.
		static HarvestingConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a harvesting configuration from \a resourcesPath.
		static HarvestingConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
