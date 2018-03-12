#pragma once
#include <boost/filesystem/path.hpp>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace zeromq {

	/// Messaging configuration settings.
	struct MessagingConfiguration {
	public:
		/// The subscriber port.
		unsigned short SubscriberPort;

	private:
		MessagingConfiguration() = default;

	public:
		/// Creates an uninitialized messaging configuration.
		static MessagingConfiguration Uninitialized();

	public:
		/// Loads a messaging configuration from \a bag.
		static MessagingConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a messaging configuration from \a resourcesPath.
		static MessagingConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
