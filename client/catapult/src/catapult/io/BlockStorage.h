#pragma once
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/NonCopyable.h"
#include <memory>

namespace catapult { namespace io {

	/// A light interface for block storage (does not allow block loading).
	class LightBlockStorage : public utils::NonCopyable {
	public:
		virtual ~LightBlockStorage() {}

	public:
		/// Gets the number of blocks.
		virtual Height chainHeight() const = 0;

		/// Returns a range of at most \a maxHashes hashes starting at \a height.
		virtual model::HashRange loadHashesFrom(Height height, size_t maxHashes) const = 0;

		/// Saves \a blockElement.
		virtual void saveBlock(const model::BlockElement& blockElement) = 0;

		/// Drops all blocks after \a height.
		virtual void dropBlocksAfter(Height height) = 0;
	};

	/// An interface for saving and loading blocks.
	class BlockStorage : public LightBlockStorage {
	public:
		/// Returns the block at \a height.
		virtual std::shared_ptr<const model::Block> loadBlock(Height height) const = 0;

		/// Returns the block element (owning a block) at \a height.
		virtual std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const = 0;
	};

	/// An interface that allows saving, loading and pruning blocks.
	class PrunableBlockStorage : public BlockStorage {
	public:
		/// Drops all blocks before \a height.
		virtual void pruneBlocksBefore(Height height) = 0;
	};
}}
