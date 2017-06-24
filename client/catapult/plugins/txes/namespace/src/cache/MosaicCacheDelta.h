#pragma once
#include "MosaicCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include <vector>

namespace catapult { namespace cache {

	/// Basic delta on top of the mosaic cache.
	class BasicMosaicCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = mosaic_cache_types::CacheReadOnlyType;

	private:
		using IdBasedHistoryBaseSetDeltaPointerType = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetDeltaPointerType;
		using NamespaceIdBasedMosaicIdsBaseSetDeltaPointerType = mosaic_cache_types::namespace_id_mosaic_ids_map::BaseSetDeltaPointerType;
		using HeightBasedMosaicIdsBaseSetDeltaPointerType = mosaic_cache_types::height_mosaic_ids_map::BaseSetDeltaPointerType;

	public:
		/// Creates a delta based on the id based history map (\a pHistoryById), the namespace id based
		/// mosaic ids map (\a pMosaicIdsByNamespaceId) and the expiry height based mosaic ids map (\a pMosaicIdsByExpiryHeight).
		explicit BasicMosaicCacheDelta(
				const IdBasedHistoryBaseSetDeltaPointerType& pHistoryById,
				const NamespaceIdBasedMosaicIdsBaseSetDeltaPointerType& pMosaicIdsByNamespaceId,
				const HeightBasedMosaicIdsBaseSetDeltaPointerType& pMosaicIdsByExpiryHeight)
				: m_pHistoryById(pHistoryById)
				, m_pMosaicIdsByNamespaceId(pMosaicIdsByNamespaceId)
				, m_pMosaicIdsByExpiryHeight(pMosaicIdsByExpiryHeight)
		{}

	public:
		/// Gets the number of mosaics in the cache.
		size_t size() const;

		/// Gets the total number of mosaics in the cache (including versions).
		size_t deepSize() const;

	public:
		/// Gets a value indicating whether or not the specified mosaic \a id is contained in the cache.
		bool contains(MosaicId id) const;

		/// Gets a value indicating whether or not a mosaic with \a id is active at \a height.
		bool isActive(MosaicId id, Height height) const;

		/// Gets a const mosaic entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		const state::MosaicEntry& get(MosaicId id) const;

		/// Gets a mutable mosaic entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		state::MosaicEntry& get(MosaicId id);

	public:
		/// Inserts the mosaic \a entry into the cache.
		void insert(const state::MosaicEntry& entry);

		/// Removes the mosaic specified by its \a id from the cache.
		void remove(MosaicId id);

		/// Removes all mosaics associated with namespace \a id from the cache.
		void remove(NamespaceId id);

		/// Prunes the mosaic cache at \a height.
		void prune(Height height);

	public:
		/// Gets all added mosaic histories.
		std::vector<const state::MosaicHistory*> addedMosaicHistories() const;

		/// Gets all modified mosaic histories.
		std::vector<const state::MosaicHistory*> modifiedMosaicHistories() const;

		/// Gets the mosaic ids of all removed mosaic histories.
		std::vector<MosaicId> removedMosaicHistories() const;

	private:
		void removeIfEmpty(const state::MosaicHistory& history);

	private:
		IdBasedHistoryBaseSetDeltaPointerType m_pHistoryById;
		NamespaceIdBasedMosaicIdsBaseSetDeltaPointerType m_pMosaicIdsByNamespaceId;
		HeightBasedMosaicIdsBaseSetDeltaPointerType m_pMosaicIdsByExpiryHeight;
	};

	/// Delta on top of the mosaic cache.
	class MosaicCacheDelta : public ReadOnlyViewSupplier<BasicMosaicCacheDelta> {
	public:
		/// Creates a delta around \a pHistoryById, \a pMosaicIdsByNamespaceId and \a pMosaicIdsByExpiryHeight.
		explicit MosaicCacheDelta(
				const mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetDeltaPointerType& pHistoryById,
				const mosaic_cache_types::namespace_id_mosaic_ids_map::BaseSetDeltaPointerType& pMosaicIdsByNamespaceId,
				const mosaic_cache_types::height_mosaic_ids_map::BaseSetDeltaPointerType& pMosaicIdsByExpiryHeight)
				: ReadOnlyViewSupplier(pHistoryById, pMosaicIdsByNamespaceId, pMosaicIdsByExpiryHeight)
		{}
	};
}}
