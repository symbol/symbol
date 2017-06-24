#include "NamespaceCacheView.h"
#include "src/model/NamespaceConstants.h"
#include <numeric>

namespace catapult { namespace cache {

	namespace {
		using NamespaceByIdMap = namespace_cache_types::namespace_id_namespace_map::BaseSetType;

		const state::Namespace& GetNamespace(const NamespaceByIdMap& namespaceById, NamespaceId id) {
			const auto* pNamespace = namespaceById.find(id);
			if (!pNamespace)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown namespace", id);

			return *pNamespace;
		}
	}

	const BasicNamespaceCacheView::HistoryValueType& BasicNamespaceCacheView::getHistory(NamespaceId id) const {
		const auto& ns = GetNamespace(m_namespaceById, id);
		const auto* pHistory = m_historyById.find(ns.rootId());
		if (!pHistory)
			CATAPULT_THROW_RUNTIME_ERROR_1("no history for root namespace found", ns.rootId());

		return *pHistory;
	}

	size_t BasicNamespaceCacheView::size() const {
		return m_historyById.size();
	}

	size_t BasicNamespaceCacheView::activeSize() const {
		size_t sum = 0;
		return std::accumulate(m_historyById.cbegin(), m_historyById.cend(), sum, [](auto value, const auto& pair) {
			return value + 1 + pair.second.numActiveRootChildren();
		});
	}

	size_t BasicNamespaceCacheView::deepSize() const {
		size_t sum = 0;
		return std::accumulate(m_historyById.cbegin(), m_historyById.cend(), sum, [](auto value, const auto& pair) {
			return value + pair.second.historyDepth() + pair.second.numAllHistoricalChildren();
		});
	}

	bool BasicNamespaceCacheView::contains(NamespaceId id) const {
		return m_namespaceById.contains(id);
	}

	bool BasicNamespaceCacheView::isActive(NamespaceId id, Height height) const {
		return contains(id) && getHistory(id).back().lifetime().isActive(height);
	}

	state::NamespaceEntry BasicNamespaceCacheView::get(NamespaceId id) const {
		const auto& ns = GetNamespace(m_namespaceById, id);
		const auto& root = m_historyById.find(ns.rootId())->back();
		return state::NamespaceEntry(ns, root);
	}
}}
