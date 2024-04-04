/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/IdentifierGroup.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// Adds \a identifier with grouping \a key to \a groupedSet.
	template<typename TGroupedSet, typename TGroupingKey, typename TIdentifier>
	void AddIdentifierWithGroup(TGroupedSet& groupedSet, const TGroupingKey& key, const TIdentifier& identifier) {
		if (!groupedSet.contains(key))
			groupedSet.insert(typename TGroupedSet::ElementType(key));

		auto groupIter = groupedSet.find(key);
		auto* pGroup = groupIter.get();
		pGroup->add(identifier);
	}

	/// Calls \a action for each value in \a set with grouping \a key according to \a groupedSet.
	template<typename TSet, typename TGroupedSet, typename TGroupingKey, typename TAction>
	void ForEachIdentifierWithGroup(TSet& set, const TGroupedSet& groupedSet, const TGroupingKey& key, TAction action) {
		auto groupIter = groupedSet.find(key);
		const auto* pGroup = groupIter.get();
		if (!pGroup)
			return;

		for (const auto& identifier : pGroup->identifiers()) {
			auto valueIter = set.find(identifier);
			auto* pValue = valueIter.get();
			if (pValue)
				action(*pValue);
		}
	}

	/// Removes \a identifier with grouping \a key from \a groupedSet.
	template<typename TGroupedSet, typename TGroupingKey, typename TIdentifier>
	void RemoveIdentifierWithGroup(TGroupedSet& groupedSet, const TGroupingKey& key, const TIdentifier& identifier) {
		auto groupIter = groupedSet.find(key);
		auto* pGroup = groupIter.get();
		if (!pGroup)
			return;

		pGroup->remove(identifier);
		if (pGroup->empty())
			groupedSet.remove(key);
	}

	/// Finds identifiers of all values in \a set (with grouped view \a groupedSet) that are deactivating at \a height.
	template<typename TSet, typename TGroupedSet, typename TIdentifiers = typename TGroupedSet::ElementType::Identifiers>
	TIdentifiers FindDeactivatingIdentifiersAtHeight(const TSet& set, const TGroupedSet& groupedSet, Height height) {
		auto groupIter = groupedSet.find(height);
		const auto* pGroup = groupIter.get();
		if (!pGroup)
			return {};

		TIdentifiers identifiers;
		for (const auto& identifier : pGroup->identifiers()) {
			auto valueIter = set.find(identifier);
			auto* pValue = valueIter.get();
			if (pValue && !pValue->isActive(height) && pValue->isActive(height - Height(1)))
				identifiers.emplace(identifier);
		}

		return identifiers;
	}
}}
