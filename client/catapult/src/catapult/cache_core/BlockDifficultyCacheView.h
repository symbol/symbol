#pragma once
#include "BlockDifficultyCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the block difficulty cache.
	class BasicBlockDifficultyCacheView
			: public utils::MoveOnly
			, public SizeMixin<BlockDifficultyCacheTypes::BaseSetType>
			, public ContainsMixin<BlockDifficultyCacheTypes::BaseSetType, BlockDifficultyCacheDescriptor>
			, public SetIterationMixin<BlockDifficultyCacheTypes::BaseSetType, BlockDifficultyCacheDescriptor> {
	public:
		using ReadOnlyView = BlockDifficultyCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a difficultyInfos and \a options.
		BasicBlockDifficultyCacheView(
				const BlockDifficultyCacheTypes::BaseSetType& difficultyInfos,
				const BlockDifficultyCacheTypes::Options& options);

	public:
		/// Gets a range object that spans \a count block difficulty infos starting at the specified \a height.
		DifficultyInfoRange difficultyInfos(Height height, size_t count) const;

	private:
		const BlockDifficultyCacheTypes::BaseSetType& m_difficultyInfos;
	};

	/// View on top of the block difficulty cache.
	class BlockDifficultyCacheView : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheView> {
	public:
		/// Creates a view around \a difficultyInfos and \a options
		explicit BlockDifficultyCacheView(
				const BlockDifficultyCacheTypes::BaseSetType& difficultyInfos,
				const BlockDifficultyCacheTypes::Options& options)
				: ReadOnlyViewSupplier(difficultyInfos, options)
		{}
	};
}}
