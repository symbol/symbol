#include "NamespaceCacheDelta.h"
#include "catapult/utils/Casting.h"
#include <numeric>
#include <unordered_set>

namespace catapult { namespace cache {

	namespace {
		using NamespaceByIdMap = NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaType;

		void AddAll(NamespaceByIdMap& namespaceById, const state::RootNamespace::Children& children) {
			for (const auto& pair : children)
				namespaceById.insert(state::Namespace(pair.second));
		}

		void RemoveAll(NamespaceByIdMap& namespaceById, const state::RootNamespace::Children& children) {
			for (const auto& pair : children)
				namespaceById.remove(pair.first);
		}
	}

	BasicNamespaceCacheDelta::BasicNamespaceCacheDelta(const NamespaceCacheTypes::BaseSetDeltaPointerType& namespaceSets)
			: NamespaceCacheDeltaMixins::Size(*namespaceSets.pPrimary)
			, NamespaceCacheDeltaMixins::Contains(*namespaceSets.pFlatMap)
			, NamespaceCacheDeltaMixins::DeltaElements(*namespaceSets.pPrimary)
			, NamespaceCacheDeltaMixins::NamespaceDeepSize(*namespaceSets.pPrimary)
			, NamespaceCacheDeltaMixins::NamespaceLookup(*namespaceSets.pPrimary, *namespaceSets.pFlatMap)
			, m_pNamespaceById(namespaceSets.pFlatMap)
			, m_pHistoryById(namespaceSets.pPrimary)
	{}

	void BasicNamespaceCacheDelta::insert(const state::RootNamespace& ns) {
		auto* pHistory = m_pHistoryById->find(ns.id());
		if (pHistory) {
			// if the owner changed, remove all of the current root's children
			if (pHistory->back().owner() != ns.owner())
				RemoveAll(*m_pNamespaceById, pHistory->back().children());

			pHistory->push_back(ns.owner(), ns.lifetime());
			return;
		}

		state::RootNamespaceHistory history(ns.id());
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
		for (const auto& pair : *m_pHistoryById)
			rootIds.insert(pair.first);

		for (auto id : rootIds) {
			auto* pHistory = m_pHistoryById->find(id);
			auto removedIds = pHistory->prune(height);
			for (auto removedId : removedIds)
				m_pNamespaceById->remove(removedId);

			if (pHistory->empty())
				m_pHistoryById->remove(pHistory->id());
		}
	}
}}
