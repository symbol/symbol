#pragma once
#include "NamespaceCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include <vector>

namespace catapult { namespace cache {

	/// Basic delta on top of the namespace cache.
	class BasicNamespaceCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = namespace_cache_types::CacheReadOnlyType;

	private:
		using IdBasedNamespaceBaseSetDelta = namespace_cache_types::namespace_id_namespace_map::BaseSetDeltaType;
		using IdBasedHistoryBaseSetDelta = namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetDeltaType;
		using HistoryValueType = namespace_cache_types::namespace_id_root_namespace_history_map::ValueType;

	public:
		/// Creates a delta based on
		/// 1) the id based namespace map (\a pNamespaceById) and
		/// 2) the id based history map (\a pHistoryById).
		explicit BasicNamespaceCacheDelta(
				const std::shared_ptr<IdBasedNamespaceBaseSetDelta>& pNamespaceById,
				const std::shared_ptr<IdBasedHistoryBaseSetDelta>& pHistoryById)
				: m_pNamespaceById(pNamespaceById)
				, m_pHistoryById(pHistoryById)
		{}

	public:
		/// Gets the number of root namespaces in the cache.
		size_t size() const;

		/// Gets the number of unique active namespaces in the cache.
		size_t activeSize() const;

		/// Gets the total number of namespaces in the cache (including versions).
		size_t deepSize() const;

	public:
		/// Gets a value indicating whether or not the specified namespace \a id is contained in the cache.
		bool contains(NamespaceId id) const;

		/// Gets a value indicating whether or not a namespace with \a id is active at \a height.
		bool isActive(NamespaceId id, Height height) const;

		/// Gets a namespace entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		state::NamespaceEntry get(NamespaceId id) const;

	public:
		/// Inserts the root namespace \a ns into the cache.
		void insert(const state::RootNamespace& ns);

		/// Inserts the namespace \a ns into the cache.
		void insert(const state::Namespace& ns);

		/// Removes the namespace specified by its \a id from the cache.
		void remove(NamespaceId id);

		/// Prunes the namespace cache at \a height.
		void prune(Height height);

	public:
		/// Gets all added root namespace histories.
		std::vector<const state::RootNamespaceHistory*> addedRootNamespaceHistories() const;

		/// Gets all modified root namespace histories.
		std::vector<const state::RootNamespaceHistory*> modifiedRootNamespaceHistories() const;

		/// Gets the namespace ids of all removed root namespace histories.
		std::vector<NamespaceId> removedRootNamespaceHistories() const;

	private:
		const HistoryValueType& getHistory(NamespaceId id) const;
		void removeRoot(NamespaceId id);
		void removeChild(const state::Namespace& ns);

	private:
		std::shared_ptr<IdBasedNamespaceBaseSetDelta> m_pNamespaceById;
		std::shared_ptr<IdBasedHistoryBaseSetDelta> m_pHistoryById;
	};

	/// Delta on top of the namespace cache.
	class NamespaceCacheDelta : public ReadOnlyViewSupplier<BasicNamespaceCacheDelta> {
	public:
		/// Creates a delta around \a pNamespaceById and \a pHistoryById.
		explicit NamespaceCacheDelta(
				const std::shared_ptr<namespace_cache_types::namespace_id_namespace_map::BaseSetDeltaType>& pNamespaceById,
				const std::shared_ptr<namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetDeltaType>& pHistoryById)
				: ReadOnlyViewSupplier(pNamespaceById, pHistoryById)
		{}
	};
}}
