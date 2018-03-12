#pragma once
#include "catapult/types.h"

namespace catapult { namespace cache { class ReadOnlyAccountStateCache; } }

namespace catapult { namespace cache {

	/// A view on top of an account state cache for retrieving importances.
	class ImportanceView {
	public:
		/// Creates a view around \a cache.
		explicit ImportanceView(const ReadOnlyAccountStateCache& cache) : m_cache(cache)
		{}

	public:
		/// Tries to populate \a importance with the importance for \a publicKey at \a height.
		bool tryGetAccountImportance(const Key& publicKey, Height height, Importance& importance) const;

		/// Gets the importance for \a publicKey at \a height or a default importance if no importance is set.
		Importance getAccountImportanceOrDefault(const Key& publicKey, Height height) const;

		/// Returns \c true if \a publicKey can harvest at \a height, given a minimum harvesting balance of
		/// \a minHarvestingBalance.
		bool canHarvest(const Key& publicKey, Height height, Amount minHarvestingBalance) const;

	private:
		const ReadOnlyAccountStateCache& m_cache;
	};
}}
