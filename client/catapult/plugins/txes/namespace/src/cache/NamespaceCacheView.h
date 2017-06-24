#pragma once
#include "NamespaceCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the namespace cache.
	class BasicNamespaceCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = namespace_cache_types::CacheReadOnlyType;

	private:
		using IdBasedNamespaceBaseSet = namespace_cache_types::namespace_id_namespace_map::BaseSetType;
		using IdBasedHistoryBaseSet = namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetType;
		using HistoryValueType = namespace_cache_types::namespace_id_root_namespace_history_map::ValueType;

	public:
		/// Creates a view around \a namespaceById and \a historyById.
		explicit BasicNamespaceCacheView(
				const IdBasedNamespaceBaseSet& namespaceById,
				const IdBasedHistoryBaseSet& historyById)
				: m_namespaceById(namespaceById)
				, m_historyById(historyById)
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
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_historyById.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_historyById.cend();
		}

	private:
		const HistoryValueType& getHistory(NamespaceId id) const;

	private:
		const IdBasedNamespaceBaseSet& m_namespaceById;
		const IdBasedHistoryBaseSet& m_historyById;
	};

	/// View on top of the namespace cache.
	class NamespaceCacheView : public ReadOnlyViewSupplier<BasicNamespaceCacheView> {
	public:
		/// Creates a view around \a namespaceById and \a historyById.
		explicit NamespaceCacheView(
				const namespace_cache_types::namespace_id_namespace_map::BaseSetType& namespaceById,
				const namespace_cache_types::namespace_id_root_namespace_history_map::BaseSetType& historyById)
				: ReadOnlyViewSupplier(namespaceById, historyById)
		{}
	};
}}
