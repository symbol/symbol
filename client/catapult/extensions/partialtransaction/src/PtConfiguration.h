#pragma once
#include "catapult/utils/FileSize.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace partialtransaction {

	/// Partial transactions configuration settings.
	struct PtConfiguration {
	public:
		/// The maximum size of a partial transactions response.
		utils::FileSize CacheMaxResponseSize;

		/// The maximum size of the partial transactions cache.
		uint32_t CacheMaxSize;

	private:
		PtConfiguration() = default;

	public:
		/// Creates an uninitialized partial transactions configuration.
		static PtConfiguration Uninitialized();

	public:
		/// Loads a partial transactions configuration from \a bag.
		static PtConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a partial transactions configuration from \a resourcesPath.
		static PtConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
