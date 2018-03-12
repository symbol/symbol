#pragma once
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/IdentifierGroup.h"

namespace catapult { namespace cache {

	/// Adds an \a identifier with with grouping \a key to \a groupedSet.
	template<typename TGroupedSet, typename TGroupingKey, typename TIdentifier>
	void AddIdentifierWithGroup(TGroupedSet& groupedSet, const TGroupingKey& key, const TIdentifier& identifier) {
		if (!groupedSet.contains(key))
			groupedSet.insert(typename TGroupedSet::ElementType(key));

		auto* pGroup = groupedSet.find(key);
		pGroup->add(identifier);
	}

	/// Calls \a action for each value in \a set with grouping \a key according to \a groupedSet.
	template<typename TSet, typename TGroupedSet, typename TGroupingKey, typename TAction>
	void ForEachIdentifierWithGroup(TSet& set, const TGroupedSet& groupedSet, const TGroupingKey& key, TAction action) {
		auto* pGroup = groupedSet.find(key);
		if (!pGroup)
			return;

		for (const auto& identifier : pGroup->identifiers()) {
			auto* pValue = set.find(identifier);
			if (pValue)
				action(*pValue);
		}
	}

	/// Removes all values in \a set with grouping \a key according to \a groupedSet.
	template<typename TSet, typename TGroupedSet, typename TGroupingKey>
	void RemoveAllIdentifiersWithGroup(TSet& set, TGroupedSet& groupedSet, const TGroupingKey& key) {
		auto* pGroup = groupedSet.find(key);
		if (!pGroup)
			return;

		for (const auto& identifier : pGroup->identifiers())
			set.remove(identifier);

		groupedSet.remove(key);
	}
}}
