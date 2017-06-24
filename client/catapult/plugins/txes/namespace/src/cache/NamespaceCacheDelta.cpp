#include "NamespaceCacheDelta.h"
#include "catapult/utils/Casting.h"
#include <numeric>
#include <unordered_set>

namespace catapult { namespace cache {

	namespace {
		using NamespaceByIdMap = namespace_cache_types::namespace_id_namespace_map::BaseSetDeltaType;

		const state::Namespace& GetNamespace(const NamespaceByIdMap& namespaceById, NamespaceId id) {
			const auto* pNamespace = namespaceById.find(id);
			if (!pNamespace)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown namespace", id);

			return *pNamespace;
		}

		void AddAll(NamespaceByIdMap& namespaceById, const state::RootNamespace::Children& children) {
			for (const auto& pair : children)
				namespaceById.insert(state::Namespace(pair.second));
		}

		void RemoveAll(NamespaceByIdMap& namespaceById, const state::RootNamespace::Children& children) {
			for (const auto& pair : children)
				namespaceById.remove(pair.first);
		}
	}

	const BasicNamespaceCacheDelta::HistoryValueType& BasicNamespaceCacheDelta::getHistory(NamespaceId id) const {
		const auto& ns = GetNamespace(*m_pNamespaceById, id);
		const auto* pHistory = utils::as_const(*m_pHistoryById).find(ns.rootId());
		if (!pHistory)
			CATAPULT_THROW_RUNTIME_ERROR_1("no history for root namespace found", ns.rootId());

		return *pHistory;
	}

	size_t BasicNamespaceCacheDelta::size() const {
		return m_pHistoryById->size();
	}

	size_t BasicNamespaceCacheDelta::activeSize() const {
		size_t sum = 0;
		return std::accumulate(m_pHistoryById->cbegin(), m_pHistoryById->cend(), sum, [](auto value, const auto& pair) {
			return value + 1 + pair.second.numActiveRootChildren();
		});
	}

	size_t BasicNamespaceCacheDelta::deepSize() const {
		size_t sum = 0;
		return std::accumulate(m_pHistoryById->cbegin(), m_pHistoryById->cend(), sum, [](auto value, const auto& pair) {
			return value + pair.second.historyDepth() + pair.second.numAllHistoricalChildren();
		});
	}

	bool BasicNamespaceCacheDelta::contains(NamespaceId id) const {
		return m_pNamespaceById->contains(id);
	}

	bool BasicNamespaceCacheDelta::isActive(NamespaceId id, Height height) const {
		return contains(id) && getHistory(id).back().lifetime().isActive(height);
	}

	state::NamespaceEntry BasicNamespaceCacheDelta::get(NamespaceId id) const {
		const auto& ns = GetNamespace(*m_pNamespaceById, id);
		const auto& root = utils::as_const(*m_pHistoryById).find(ns.rootId())->back();
		return state::NamespaceEntry(ns, root);
	}

	void BasicNamespaceCacheDelta::insert(const state::RootNamespace& ns) {
		auto* pHistory = m_pHistoryById->find(ns.id());
		if (pHistory) {
			// if the owner changed, remove all of the current root's children
			if (pHistory->back().owner() != ns.owner())
				RemoveAll(*m_pNamespaceById, pHistory->back().children());

			pHistory->push_back(ns.owner(), ns.lifetime());
			return;
		}

		HistoryValueType history(ns.id());
		history.push_back(ns.owner(), ns.lifetime());
		m_pHistoryById->insert(std::move(history));

		state::Namespace::Path path;
		path.push_back(ns.id());
		m_pNamespaceById->insert(state::Namespace(path));
	}

	void BasicNamespaceCacheDelta::insert(const state::Namespace& ns) {
		auto* pHistory = m_pHistoryById->find(ns.rootId());
		if (!pHistory)
			CATAPULT_THROW_INVALID_ARGUMENT_1("no root namespace exists for namespace", ns.id());

		pHistory->back().add(ns);
		m_pNamespaceById->insert(ns);
	}

	void BasicNamespaceCacheDelta::remove(NamespaceId id) {
		const auto* pNamespace = m_pNamespaceById->find(id);
		if (!pNamespace)
			CATAPULT_THROW_INVALID_ARGUMENT_1("no namespace exists", id);

		if (pNamespace->rootId() == id)
			removeRoot(id);
		else
			removeChild(*pNamespace);
	}

	void BasicNamespaceCacheDelta::removeRoot(NamespaceId id) {
		auto* pHistory = m_pHistoryById->find(id);
		if (1 == pHistory->historyDepth() && !pHistory->back().empty())
			CATAPULT_THROW_RUNTIME_ERROR_1("cannot remove non-empty root namespace", id);

		// make a copy of the current root and remove it
		auto removedRoot = pHistory->back();
		pHistory->pop_back();
		if (pHistory->empty()) {
			// note that the last root in the history is always empty when getting removed
			m_pHistoryById->remove(id);
			m_pNamespaceById->remove(id);
			return;
		}

		// if the owner changed, update all children so that only the new root's children are contained
		if (removedRoot.owner() != pHistory->back().owner()) {
			RemoveAll(*m_pNamespaceById, removedRoot.children());
			AddAll(*m_pNamespaceById, pHistory->back().children());
		}
	}

	void BasicNamespaceCacheDelta::removeChild(const state::Namespace& ns) {
		auto* pHistory = m_pHistoryById->find(ns.rootId());
		pHistory->back().remove(ns.id());
		m_pNamespaceById->remove(ns.id());
	}

	void BasicNamespaceCacheDelta::prune(Height height) {
		std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>> rootIds;
		for (auto iter = m_pHistoryById->cbegin(); m_pHistoryById->cend() != iter; ++iter)
			rootIds.insert(iter->first);

		for (auto id : rootIds) {
			auto* pHistory = m_pHistoryById->find(id);
			auto removedIds = pHistory->prune(height);
			for (auto removedId : removedIds)
				m_pNamespaceById->remove(removedId);

			if (pHistory->empty())
				m_pHistoryById->remove(pHistory->id());
		}
	}

	namespace {
		template<typename TDestination, typename TSource>
		void CollectAll(TDestination& dest, const TSource& source) {
			for (const auto& pair : source)
				dest.push_back(&pair.second);
		}
	}

	std::vector<const state::RootNamespaceHistory*> BasicNamespaceCacheDelta::addedRootNamespaceHistories() const {
		std::vector<const state::RootNamespaceHistory*> addedHistories;
		auto deltas = m_pHistoryById->deltas();
		CollectAll(addedHistories, deltas.Added);
		return addedHistories;
	}

	std::vector<const state::RootNamespaceHistory*> BasicNamespaceCacheDelta::modifiedRootNamespaceHistories() const {
		std::vector<const state::RootNamespaceHistory*> modifiedHistories;
		auto deltas = m_pHistoryById->deltas();
		CollectAll(modifiedHistories, deltas.Copied);
		return modifiedHistories;
	}

	std::vector<NamespaceId> BasicNamespaceCacheDelta::removedRootNamespaceHistories() const {
		std::vector<NamespaceId> removedHistories;
		auto deltas = m_pHistoryById->deltas();
		for (const auto& pair : deltas.Removed)
			removedHistories.push_back(pair.first);

		return removedHistories;
	}
}}
