#pragma once

namespace catapult { namespace deltaset {

	/// Slim wrapper around changed entities.
	template<typename TSet>
	struct DeltaEntities {
	public:
		/// Creates new delta entities from \a addedEntities, \a removedEntities, and \a copiedEntities.
		constexpr explicit DeltaEntities(const TSet& addedEntities, const TSet& removedEntities, const TSet& copiedEntities)
				: Added(addedEntities)
				, Removed(removedEntities)
				, Copied(copiedEntities)
		{}

	public:
		/// Added entities.
		const TSet& Added;

		/// Removed entities.
		const TSet& Removed;

		/// Copied entities.
		const TSet& Copied;
	};
}}
