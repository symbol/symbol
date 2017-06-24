#pragma once
#include <algorithm>

namespace catapult { namespace deltaset {

	/// Searches for any of the given \a elements in the container (\a pContainer).
	/// Returns \c true if any element is found or \c false if none is found.
	template<typename TContainer, typename TElements>
	bool ContainsAny(const TContainer& container, const TElements& elements) {
		return std::any_of(elements.cbegin(), elements.cend(), [&container](const auto& pElement) {
			return container.contains(pElement);
		});
	}

	/// Inserts all \a elements into the container (\a pContainer).
	/// The algorithm relies on the data used for comparing elements being immutable.
	template<typename TContainer, typename TElements>
	void InsertAll(TContainer& container, const TElements& elements) {
		for (const auto& pElement : elements)
			container.insert(pElement);
	}

	/// Removes all \a elements from the container (\a pContainer).
	template<typename TContainer, typename TElements>
	void RemoveAll(TContainer& container, const TElements& elements) {
		for (const auto& pElement : elements)
			container.remove(pElement);
	}
}}
