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

#include "MockMemoryBlockStorage.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"

namespace catapult { namespace mocks {

	// region MockMemoryBlockStorage

	namespace {
		model::BlockElement CreateNemesisBlockElement() {
			return test::BlockToBlockElement(test::GetNemesisBlock(), test::GetNemesisGenerationHashSeed());
		}
	}

	MockMemoryBlockStorage::MockMemoryBlockStorage() : MemoryBlockStorage(CreateNemesisBlockElement())
	{}

	// endregion

	// region factories

	namespace {
		template<typename TStorage>
		void SaveBlocks(TStorage& storage, uint32_t numBlocks) {
			// storage already contains nemesis block (height 1)
			for (auto i = 2u; i <= numBlocks; ++i) {
				model::Block block;
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&block), sizeof(model::BlockHeader) });

				block.Size = sizeof(model::BlockHeader);
				block.Height = Height(i);
				storage.saveBlock(test::BlockToBlockElement(block));
			}
		}
	}

	std::unique_ptr<io::PrunableBlockStorage> CreateMemoryBlockStorage(uint32_t numBlocks) {
		auto pStorage = std::make_unique<MockMemoryBlockStorage>();
		SaveBlocks(*pStorage, numBlocks);
		return PORTABLE_MOVE(pStorage);
	}

	std::unique_ptr<io::BlockStorageCache> CreateMemoryBlockStorageCache(uint32_t numBlocks) {
		return std::make_unique<io::BlockStorageCache>(CreateMemoryBlockStorage(numBlocks), CreateMemoryBlockStorage(0));
	}

	// endregion

	// region utils

	void SeedStorageWithFixedSizeBlocks(io::BlockStorageCache& storage, uint32_t numBlocks) {
		auto modifier = storage.modifier();
		SaveBlocks(modifier, numBlocks);
		modifier.commit();
	}

	// endregion
}}
