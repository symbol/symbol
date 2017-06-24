#pragma once
#include "MosaicCacheDelta.h"
#include "MosaicCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of mosaic information.
	class BasicMosaicCache : public utils::MoveOnly {
	public:
		using CacheViewType = MosaicCacheView;
		using CacheDeltaType = MosaicCacheDelta;
		using CacheReadOnlyType = mosaic_cache_types::CacheReadOnlyType;

	private:
		// history map
		using IdBasedHistoryBaseSetDeltaPointerType = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetDeltaPointerType;
		using IdBasedHistoryBaseSet = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetType;

		// namespace id based mosaic ids map
		using NamespaceIdBasedMosaicIdsBaseSetDeltaPointerType = mosaic_cache_types::namespace_id_mosaic_ids_map::BaseSetDeltaPointerType;
		using NamespaceIdBasedMosaicIdsBaseSet = mosaic_cache_types::namespace_id_mosaic_ids_map::BaseSetType;

		// height based mosaic ids map
		using HeightBasedMosaicIdsBaseSetDeltaPointerType = mosaic_cache_types::height_mosaic_ids_map::BaseSetDeltaPointerType;
		using HeightBasedMosaicIdsBaseSet = mosaic_cache_types::height_mosaic_ids_map::BaseSetType;

	public:
		/// Creates a mosaic cache.
		BasicMosaicCache() = default;

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_historyById);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_historyById.rebase(), m_mosaicIdsByNamespaceId.rebase(), m_mosaicIdsByExpiryHeight.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(
					m_historyById.rebaseDetached(),
					m_mosaicIdsByNamespaceId.rebaseDetached(),
					m_mosaicIdsByExpiryHeight.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType&) {
			m_historyById.commit();
			m_mosaicIdsByNamespaceId.commit();
			m_mosaicIdsByExpiryHeight.commit();
		}

	private:
		inline CacheDeltaType createDelta(
				const IdBasedHistoryBaseSetDeltaPointerType& pHistoryById,
				const NamespaceIdBasedMosaicIdsBaseSetDeltaPointerType& pMosaicIdsByNamespaceIds,
				const HeightBasedMosaicIdsBaseSetDeltaPointerType& pMosaicIdsByExpiryHeight) const {
			return CacheDeltaType(pHistoryById, pMosaicIdsByNamespaceIds, pMosaicIdsByExpiryHeight);
		}

	private:
		IdBasedHistoryBaseSet m_historyById;
		NamespaceIdBasedMosaicIdsBaseSet m_mosaicIdsByNamespaceId;
		HeightBasedMosaicIdsBaseSet m_mosaicIdsByExpiryHeight;
	};

	/// Synchronized cache composed of mosaic information.
	class MosaicCache : public SynchronizedCache<BasicMosaicCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 4;

		/// The cache friendly name.
		static constexpr auto Name = "MosaicCache";

	public:
		/// Creates a mosaic cache.
		explicit MosaicCache() : SynchronizedCache<BasicMosaicCache>(BasicMosaicCache())
		{}
	};
}}
