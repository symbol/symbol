#include "RootNamespaceHistory.h"
#include "catapult/state/AccountState.h"

namespace catapult { namespace state {
	namespace {
		void AddAllIds(std::set<NamespaceId>& ids, const RootNamespace::Children& children) {
			for (const auto& pair : children)
				ids.insert(pair.first);
		}

		void RemoveAllIds(std::set<NamespaceId>& ids, const RootNamespace::Children& children) {
			for (const auto& pair : children)
				ids.erase(pair.first);
		}
	}

	void RootNamespaceHistory::push_back(const Key& owner, const NamespaceLifetime& lifetime) {
		if (!m_rootHistory.empty()) {
			const auto& previousNamespace = back();
			if (previousNamespace.owner() == owner) {
				// inherit all children since it is the same owner
				m_rootHistory.push_back(previousNamespace.renew(lifetime));
				return;
			}
		}

		m_rootHistory.emplace_back(m_id, owner, lifetime);
	}

	void RootNamespaceHistory::pop_back() {
		m_rootHistory.pop_back();
	}

	std::set<NamespaceId> RootNamespaceHistory::prune(Height height) {
		std::set<NamespaceId> ids;
		auto expiredPredicate = [height](const auto& entry) {
			return entry.lifetime().End <= height;
		};

		for (auto iter = m_rootHistory.begin(); m_rootHistory.end() != iter;) {
			if (expiredPredicate(*iter)) {
				AddAllIds(ids, iter->children());
				iter = m_rootHistory.erase(iter);
			} else {
				RemoveAllIds(ids, iter->children());
				++iter;
			}
		}

		if (m_rootHistory.empty())
			ids.insert(m_id);

		return ids;
	}
}}
