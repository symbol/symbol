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

#include "catapult/chain/FinalizationUtils.h"
#include "catapult/io/BlockStorageCache.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockBlockStorage.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationUtilsTests

	namespace {
		// region MockFinalizedChainHeightBlockStorage

		class MockFinalizedChainHeightBlockStorage : public mocks::UnsupportedBlockStorage {
		public:
			MockFinalizedChainHeightBlockStorage(Height chainHeight, Height finalizedChainHeight)
					: m_chainHeight(chainHeight)
					, m_finalizedChainHeight(finalizedChainHeight)
					, m_pMemoryBlockStorage(mocks::CreateMemoryBlockStorage(0))
			{}

		public:
			Height chainHeight() const override {
				return m_chainHeight;
			}

			Height finalizedChainHeight() const override {
				return m_finalizedChainHeight;
			}

			// loadBlockElement needs to be implemented because BlockStorageCache loads the tail block into its cache
			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				model::Block block;
				block.Size = sizeof(model::BlockHeader);
				block.Height = m_chainHeight;

				// reset memory storage to only contain single block at height
				m_pMemoryBlockStorage->dropBlocksAfter(m_chainHeight - Height(1));
				m_pMemoryBlockStorage->saveBlock(test::BlockToBlockElement(block));
				return m_pMemoryBlockStorage->loadBlockElement(height);
			}

		private:
			Height m_chainHeight;
			Height m_finalizedChainHeight;
			std::shared_ptr<io::BlockStorage> m_pMemoryBlockStorage;
		};

		// endregion

		Height GetFinalizedChainHeight(Height chainHeight, Height finalizedChainHeight, uint32_t maxRollbackBlocks) {
			auto storage = io::BlockStorageCache(
					std::make_unique<MockFinalizedChainHeightBlockStorage>(chainHeight, finalizedChainHeight),
					mocks::CreateMemoryBlockStorage(0));
			return chain::GetFinalizedChainHeight(storage.view(), maxRollbackBlocks);
		}
	}

	TEST(TEST_CLASS, GetFinalizedChainHeight_UsesChainHeightFromStorageWhenFinalizationDisabled_SameHeights) {
		constexpr auto Max_Rollback_Blocks = 25u;
		EXPECT_EQ(Height(1), GetFinalizedChainHeight(Height(1), Height(1), Max_Rollback_Blocks));
		EXPECT_EQ(Height(1), GetFinalizedChainHeight(Height(25), Height(25), Max_Rollback_Blocks));
		EXPECT_EQ(Height(1), GetFinalizedChainHeight(Height(26), Height(26), Max_Rollback_Blocks));
		EXPECT_EQ(Height(2), GetFinalizedChainHeight(Height(27), Height(27), Max_Rollback_Blocks));

		EXPECT_EQ(Height(25), GetFinalizedChainHeight(Height(50), Height(50), Max_Rollback_Blocks));
		EXPECT_EQ(Height(50), GetFinalizedChainHeight(Height(75), Height(75), Max_Rollback_Blocks));
	}

	TEST(TEST_CLASS, GetFinalizedChainHeight_UsesChainHeightFromStorageWhenFinalizationDisabled_DifferentHeights) {
		constexpr auto Max_Rollback_Blocks = 25u;
		EXPECT_EQ(Height(25), GetFinalizedChainHeight(Height(50), Height(75), Max_Rollback_Blocks));
		EXPECT_EQ(Height(50), GetFinalizedChainHeight(Height(75), Height(50), Max_Rollback_Blocks));
	}

	TEST(TEST_CLASS, GetFinalizedChainHeight_UsesFinalizedChainHeightFromStorageWhenFinalizationEnabled_SameHeights) {
		constexpr auto Max_Rollback_Blocks = 0u;
		EXPECT_EQ(Height(1), GetFinalizedChainHeight(Height(1), Height(1), Max_Rollback_Blocks));
		EXPECT_EQ(Height(25), GetFinalizedChainHeight(Height(25), Height(25), Max_Rollback_Blocks));
		EXPECT_EQ(Height(26), GetFinalizedChainHeight(Height(26), Height(26), Max_Rollback_Blocks));
		EXPECT_EQ(Height(27), GetFinalizedChainHeight(Height(27), Height(27), Max_Rollback_Blocks));

		EXPECT_EQ(Height(50), GetFinalizedChainHeight(Height(50), Height(50), Max_Rollback_Blocks));
		EXPECT_EQ(Height(75), GetFinalizedChainHeight(Height(75), Height(75), Max_Rollback_Blocks));
	}

	TEST(TEST_CLASS, GetFinalizedChainHeight_UsesFinalizedChainHeightFromStorageWhenFinalizationEnabled_DifferentHeights) {
		constexpr auto Max_Rollback_Blocks = 0u;
		EXPECT_EQ(Height(50), GetFinalizedChainHeight(Height(75), Height(50), Max_Rollback_Blocks));
		EXPECT_EQ(Height(75), GetFinalizedChainHeight(Height(50), Height(75), Max_Rollback_Blocks));
	}
}}
