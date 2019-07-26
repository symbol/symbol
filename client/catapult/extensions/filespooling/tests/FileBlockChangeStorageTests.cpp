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

#include "filespooling/src/FileBlockChangeStorage.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FileBlockChangeStorageTests

	namespace {
		constexpr auto Empty_Block_Element_Size = sizeof(model::BlockHeader) + 2 * Hash256::Size + 2 * sizeof(uint32_t);

		template<typename TAction>
		void RunTest(TAction action) {
			// Arrange: create output stream
			std::vector<uint8_t> buffer;
			auto pStream = std::make_unique<mocks::MockMemoryStream>(buffer);
			const auto& stream = *pStream;

			// - create storage
			auto pStorage = CreateFileBlockChangeStorage(std::move(pStream));

			// Act:
			action(*pStorage, stream, buffer);
		}
	}

	TEST(TEST_CLASS, NotifyBlockWritesToUnderlyingStream_BlockWithoutStatements) {
		// Arrange:
		RunTest([](auto& storage, const auto& stream, const auto& buffer) {
			auto pBlock = test::GenerateEmptyRandomBlock();
			model::BlockElement blockElement(*pBlock);

			// Act:
			storage.notifyBlock(blockElement);

			// Assert:
			EXPECT_EQ(1u, stream.numFlushes());
			ASSERT_EQ(1u + Empty_Block_Element_Size + 1, buffer.size());

			auto expectedOperationType = subscribers::BlockChangeOperationType::Block;
			EXPECT_EQ(expectedOperationType, static_cast<subscribers::BlockChangeOperationType>(buffer[0]));

			// - spot check block part of element
			EXPECT_EQ(*pBlock, reinterpret_cast<const model::Block&>(buffer[1]));

			// - block statement was not written
			EXPECT_EQ(0u, buffer[1u + Empty_Block_Element_Size]);
		});
	}

	TEST(TEST_CLASS, NotifyBlockWritesToUnderlyingStream_BlockWithStatements) {
		// Arrange:
		RunTest([](auto& storage, const auto& stream, const auto& buffer) {
			auto pBlock = test::GenerateEmptyRandomBlock();
			model::BlockElement blockElement(*pBlock);
			blockElement.OptionalStatement = std::make_shared<model::BlockStatement>();

			// Act:
			storage.notifyBlock(blockElement);

			// Assert:
			EXPECT_EQ(1u, stream.numFlushes());
			ASSERT_EQ(1u + Empty_Block_Element_Size + 1 + 3 * sizeof(uint32_t), buffer.size());

			auto expectedOperationType = subscribers::BlockChangeOperationType::Block;
			EXPECT_EQ(expectedOperationType, static_cast<subscribers::BlockChangeOperationType>(buffer[0]));

			// - spot check block part of element
			EXPECT_EQ(*pBlock, reinterpret_cast<const model::Block&>(buffer[1]));

			// - empty block statement was written
			EXPECT_EQ(0xFFu, buffer[1u + Empty_Block_Element_Size]);

			const auto* pStatementCounts = reinterpret_cast<const uint32_t*>(&buffer[1u + Empty_Block_Element_Size + 1]);
			EXPECT_EQ(0u, pStatementCounts[0]);
			EXPECT_EQ(0u, pStatementCounts[1]);
			EXPECT_EQ(0u, pStatementCounts[2]);
		});
	}

	TEST(TEST_CLASS, NotifyDropBlocksAfterWritesToUnderlyingStream) {
		// Arrange:
		RunTest([](auto& storage, const auto& stream, const auto& buffer) {
			// Act:
			storage.notifyDropBlocksAfter(Height(246));

			// Assert:
			EXPECT_EQ(1u, stream.numFlushes());
			ASSERT_EQ(1u + sizeof(uint64_t), buffer.size());

			auto expectedOperationType = subscribers::BlockChangeOperationType::Drop_Blocks_After;
			EXPECT_EQ(expectedOperationType, static_cast<subscribers::BlockChangeOperationType>(buffer[0]));

			EXPECT_EQ(Height(246), reinterpret_cast<const Height&>(buffer[1]));
		});
	}
}}
