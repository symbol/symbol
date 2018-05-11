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
#include "catapult/io/BlockStorage.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/Elements.h"
#include <map>

namespace catapult { namespace mocks {

	extern const unsigned char MemoryBasedStorage_NemesisBlockData[];

	/// A mock memory-based block storage that loads and saves blocks in memory.
	class MockMemoryBasedStorage final : public io::BlockStorage {
	private:
		using Blocks = std::map<Height, std::shared_ptr<model::Block>>;
		using BlockElements = std::map<Height, std::shared_ptr<model::BlockElement>>;

	public:
		/// Creates a mock memory-based block storage.
		MockMemoryBasedStorage();

	public:
		Height chainHeight() const override;

	public:
		std::shared_ptr<const model::Block> loadBlock(Height height) const override;
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override;
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;

		void saveBlock(const model::BlockElement& blockElement) override;
		void dropBlocksAfter(Height height) override;

	private:
		Blocks m_blocks;
		BlockElements m_blockElements;
		Height m_height;
	};

	/// Creates a memory based storage composed of \a numBlocks.
	std::unique_ptr<io::BlockStorage> CreateMemoryBasedStorage(uint32_t numBlocks);

	/// Creates a memory based storage cache composed of \a numBlocks.
	std::unique_ptr<io::BlockStorageCache> CreateMemoryBasedStorageCache(uint32_t numBlocks);
}}
