#pragma once
#include "src/state/NamespaceEntry.h"
#include "src/state/RootNamespaceHistory.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include <numeric>

namespace catapult { namespace cache {

	/// A mixin for calculating the deep size of namespaces.
	template<typename TSet>
	class NamespaceDeepSizeMixin {
	public:
		/// Creates a mixin around \a set.
		explicit NamespaceDeepSizeMixin(const TSet& set) : m_set(set)
		{}

	public:
		/// Gets the number of unique active namespaces in the cache.
		size_t activeSize() const {
			size_t sum = 0;
			return std::accumulate(m_set.begin(), m_set.end(), sum, [](auto value, const auto& pair) {
				return value + 1 + pair.second.numActiveRootChildren();
			});
		}

		/// Gets the total number of namespaces in the cache (including versions).
		size_t deepSize() const {
			size_t sum = 0;
			return std::accumulate(m_set.begin(), m_set.end(), sum, [](auto value, const auto& pair) {
				return value + pair.second.historyDepth() + pair.second.numAllHistoricalChildren();
			});
		}

	private:
		const TSet& m_set;
	};

	/// A mixin for looking up namespaces.
	/// \note Due to double lookups, this cannot be replaced with typical ActivePredicateMixin and ConstPredicateMixin.
	template<typename TPrimarySet, typename TFlatMap>
	class NamespaceLookupMixin {
	public:
		/// Creates a mixin around (history by id) \a set and \a flatMap.
		explicit NamespaceLookupMixin(const TPrimarySet& set, const TFlatMap& flatMap)
				: m_set(set)
				, m_flatMap(flatMap)
		{}

	public:
		/// Returns \c true if the value specified by identifier \a id is active at \a height.
		bool isActive(NamespaceId id, Height height) const {
			return m_flatMap.contains(id) && getHistory(id).back().lifetime().isActive(height);
		}

		/// Gets a value specified by identifier \a id.
		/// \throws catapult_invalid_argument if the requested value is not found.
		state::NamespaceEntry get(NamespaceId id) const {
			const auto& ns = getNamespace(id);
			const auto& root = m_set.find(ns.rootId())->back();
			return state::NamespaceEntry(ns, root);
		}

	private:
		const state::Namespace& getNamespace(NamespaceId id) const {
			const auto* pNamespace = m_flatMap.find(id);
			if (!pNamespace)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown namespace", id);

			return *pNamespace;
		}

		const state::RootNamespaceHistory& getHistory(NamespaceId id) const {
			const auto& ns = getNamespace(id);
			const auto* pHistory = m_set.find(ns.rootId());
			if (!pHistory)
				CATAPULT_THROW_RUNTIME_ERROR_1("no history for root namespace found", ns.rootId());

			return *pHistory;
		}

	private:
		const TPrimarySet& m_set;
		const TFlatMap& m_flatMap;
	};
}}
