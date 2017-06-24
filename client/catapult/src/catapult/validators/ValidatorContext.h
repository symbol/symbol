#pragma once
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/types.h"
#include <cstdint>
#include <limits>

namespace catapult { namespace validators {

	/// Contextual information passed to stateful validators.
	struct ValidatorContext {
	public:
		/// Creates a validator context around a \a height, \a blockTime, \a network and \a cache.
		constexpr ValidatorContext(
				catapult::Height height,
				Timestamp blockTime,
				const model::NetworkInfo& network,
				const cache::ReadOnlyCatapultCache& cache)
				: Height(height)
				, BlockTime(blockTime)
				, Network(network)
				, Cache(cache)
		{}

	public:
		/// The current height.
		const catapult::Height Height;

		/// The current block time.
		const Timestamp BlockTime;

		/// The network info.
		const model::NetworkInfo Network;

		/// The catapult cache.
		const cache::ReadOnlyCatapultCache& Cache;
	};
}}
