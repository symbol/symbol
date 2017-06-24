#include "MosaicHistory.h"
#include "MosaicEntry.h"

namespace catapult { namespace state {

	void MosaicHistory::push_back(const MosaicDefinition& definition, Amount supply) {
		auto entry = MosaicEntry(m_namespaceId, m_id, definition);
		entry.increaseSupply(supply);
		m_history.emplace_back(entry);
	}

	void MosaicHistory::pop_back() {
		m_history.pop_back();
	}

	void MosaicHistory::prune(Height height) {
		auto expiredPredicate = [height](const auto& entry) {
			return entry.definition().isExpired(height);
		};

		// reverse iterate through the list
		// from the first expired entry downward, remove all entries
		for (auto iter = m_history.rbegin(); m_history.rend() != iter;) {
			if (expiredPredicate(*iter)) {
				m_history.erase(m_history.begin(), ++std::next(iter).base());
				break;
			}

			++iter;
		}
	}
}}
