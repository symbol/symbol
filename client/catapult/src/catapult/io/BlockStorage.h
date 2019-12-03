/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/NonCopyable.h"
#include <memory>

namespace catapult { namespace io {

	/// Minimalistic interface for block storage (does not allow block loading).
	class LightBlockStorage : public utils::NonCopyable {
	public:
		virtual ~LightBlockStorage() = default;

	public:
		/// Gets the number of blocks.
		virtual Height chainHeight() const = 0;

		/// Gets a range of at most \a maxHashes hashes starting at \a height.
		virtual model::HashRange loadHashesFrom(Height height, size_t maxHashes) const = 0;

		/// Saves \a blockElement.
		virtual void saveBlock(const model::BlockElement& blockElement) = 0;

		/// Drops all blocks after \a height.
		virtual void dropBlocksAfter(Height height) = 0;
	};

	/// Interface for saving and loading blocks.
	class BlockStorage : public LightBlockStorage {
	public:
		/// Gets the block at \a height.
		virtual std::shared_ptr<const model::Block> loadBlock(Height height) const = 0;

		/// Gets the block element (owning a block) at \a height.
		virtual std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const = 0;

		/// Gets the optional block statement data at \a height.
		virtual std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const = 0;
	};

	/// Interface that allows saving, loading and pruning blocks.
	class PLUGIN_API_DEPENDENCY PrunableBlockStorage : public BlockStorage {
	public:
		/// Purges all blocks from storage.
		virtual void purge() = 0;
	};
}}
