#pragma once
#include "MosaicCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the mosaic cache.
	class BasicMosaicCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = mosaic_cache_types::CacheReadOnlyType;

	private:
		using IdBasedHistoryBaseSet = mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetType;

	public:
		/// Creates a view around \a historyById.
		explicit BasicMosaicCacheView(const IdBasedHistoryBaseSet& historyById) : m_historyById(historyById)
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

		/// Gets a mosaic entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		const state::MosaicEntry& get(MosaicId id) const;

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
		const IdBasedHistoryBaseSet& m_historyById;
	};

	/// View on top of the mosaic cache.
	class MosaicCacheView : public ReadOnlyViewSupplier<BasicMosaicCacheView> {
	public:
		/// Creates a view around \a historyById.
		explicit MosaicCacheView(const mosaic_cache_types::mosaic_id_mosaic_history_map::BaseSetType& historyById)
				: ReadOnlyViewSupplier(historyById)
		{}
	};
}}
