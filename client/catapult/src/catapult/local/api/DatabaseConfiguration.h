#pragma once
#include <boost/filesystem/path.hpp>
#include <string>
#include <unordered_set>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace local { namespace api {

	/// The database configuration settings.
	struct DatabaseConfiguration {
	public:
		/// The database uri.
		std::string DatabaseUri;

		/// The database name.
		std::string DatabaseName;

		/// The maximum number of database writer threads.
		uint32_t MaxWriterThreads;

		/// Names of the database plugins that should be loaded.
		std::unordered_set<std::string> PluginNames;

	private:
		DatabaseConfiguration() = default;

	public:
		/// Creates an uninitialized database configuration.
		static DatabaseConfiguration Uninitialized();

	public:
		/// Loads a database configuration from \a bag.
		static DatabaseConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a database configuration from \a resourcesPath.
		static DatabaseConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}}
