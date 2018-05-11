/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "MosaicHistory.h"
#include "MosaicEntry.h"

namespace catapult { namespace state {

	MosaicHistory::MosaicHistory(NamespaceId namespaceId, MosaicId id) : m_namespaceId(namespaceId), m_id(id)
	{}

	bool MosaicHistory::empty() const {
		return m_history.empty();
	}

	NamespaceId MosaicHistory::namespaceId() const {
		return m_namespaceId;
	}

	MosaicId MosaicHistory::id() const {
		return m_id;
	}

	size_t MosaicHistory::historyDepth() const {
		return m_history.size();
	}

	void MosaicHistory::push_back(const MosaicDefinition& definition, Amount supply) {
		auto entry = MosaicEntry(m_namespaceId, m_id, definition);
		entry.increaseSupply(supply);
		m_history.emplace_back(entry);
	}

	void MosaicHistory::pop_back() {
		m_history.pop_back();
	}

	const MosaicEntry& MosaicHistory::back() const {
		return m_history.back();
	}

	MosaicEntry& MosaicHistory::back() {
		return m_history.back();
	}

	size_t MosaicHistory::prune(Height height) {
		auto expiredPredicate = [height](const auto& entry) {
			return entry.definition().isExpired(height);
		};

		// reverse iterate through the list
		// from the first expired entry downward, remove all entries
		auto numErasedHistories = m_history.size();
		for (auto iter = m_history.rbegin(); m_history.rend() != iter;) {
			if (expiredPredicate(*iter)) {
				m_history.erase(m_history.begin(), ++std::next(iter).base());
				break;
			}

			--numErasedHistories;
			++iter;
		}

		return numErasedHistories;
	}

	std::list<MosaicEntry>::const_iterator MosaicHistory::begin() const {
		return m_history.cbegin();
	}

	std::list<MosaicEntry>::const_iterator MosaicHistory::end() const {
		return m_history.cend();
	}

	bool MosaicHistory::isActive(Height height) const {
		return !empty() && back().definition().isActive(height);
	}
}}
