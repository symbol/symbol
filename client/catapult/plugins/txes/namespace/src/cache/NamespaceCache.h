#pragma once
#include "NamespaceCacheDelta.h"
#include "NamespaceCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of namespace information.
	class BasicNamespaceCache : public utils::MoveOnly {
	public:
		using CacheViewType = NamespaceCacheView;
		using CacheDeltaType = NamespaceCacheDelta;
		using CacheReadOnlyType = namespace_cache_types::CacheReadOnlyType;

	private:
		// namespace map
		using IdBasedNamespaceBaseSetDelta = namespace_cache_types::namespace_id_namespace_map::BaseSetDeltaType;
		using IdBasedNamespaceBaseSet = namespace_cache_types::namespace_id_namespace_map::BaseSetType;

		// history map
		using IdBasedHistoryBaseSetDelta = namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetDeltaType;
		using IdBasedHistoryBaseSet = namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetType;

	public:
		/// Creates a namespace cache.
		BasicNamespaceCache() = default;

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_namespaceById, m_historyById);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_namespaceById.rebase(), m_historyById.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(m_namespaceById.rebaseDetached(), m_historyById.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType&) {
			m_namespaceById.commit();
			m_historyById.commit();
		}

	private:
		inline CacheDeltaType createDelta(
				const std::shared_ptr<IdBasedNamespaceBaseSetDelta>& pNamespaceById,
				const std::shared_ptr<IdBasedHistoryBaseSetDelta>& pHistoryById) const {
			return CacheDeltaType(pNamespaceById, pHistoryById);
		}

	private:
		IdBasedNamespaceBaseSet m_namespaceById;
		IdBasedHistoryBaseSet m_historyById;
	};

	/// Synchronized cache composed of namespace information.
	class NamespaceCache : public SynchronizedCache<BasicNamespaceCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 3;

		/// The cache friendly name.
		static constexpr auto Name = "NamespaceCache";

	public:
		/// Creates a namespace cache.
		explicit NamespaceCache() : SynchronizedCache<BasicNamespaceCache>(BasicNamespaceCache())
		{}
	};
}}
