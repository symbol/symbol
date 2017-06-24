#pragma once
#include <stdint.h>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Transfer plugin configuration settings.
	struct TransferConfiguration {
	public:
		/// The maximum transaction message size.
		uint16_t MaxMessageSize;

	private:
		TransferConfiguration() = default;

	public:
		/// Creates an uninitialized transfer configuration.
		static TransferConfiguration Uninitialized();

		/// Loads a transfer configuration from \a bag.
		static TransferConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
