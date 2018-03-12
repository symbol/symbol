#pragma once

namespace catapult { namespace deltaset {

	/// Slim wrapper around changed elements.
	template<typename TSet>
	struct DeltaElements {
	public:
		/// Creates new delta elements from \a addedElements, \a removedElements and \a copiedElements.
		constexpr explicit DeltaElements(const TSet& addedElements, const TSet& removedElements, const TSet& copiedElements)
				: Added(addedElements)
				, Removed(removedElements)
				, Copied(copiedElements)
		{}

	public:
		/// Added elements.
		const TSet& Added;

		/// Removed elements.
		const TSet& Removed;

		/// Copied elements.
		const TSet& Copied;
	};
}}
