#pragma once
#include "catapult/utils/BlockSpan.h"
#include "catapult/types.h"
#include <stdint.h>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Lock plugin configuration settings.
	struct LockConfiguration {
	public:
		/// The amount that has to be locked per aggregate in partial cache.
		Amount LockedFundsPerAggregate;

		/// The maximum number of blocks for which a hash lock can exist.
		utils::BlockSpan MaxHashLockDuration;

		/// The maximum number of blocks for which a secret lock can exist.
		utils::BlockSpan MaxSecretLockDuration;

		/// The minimum size of a proof in bytes.
		uint16_t MinProofSize;

		/// The maximum size of a proof in bytes.
		uint16_t MaxProofSize;

	private:
		LockConfiguration() = default;

	public:
		/// Creates an uninitialized lock configuration.
		static LockConfiguration Uninitialized();

		/// Loads lock configuration from \a bag.
		static LockConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
