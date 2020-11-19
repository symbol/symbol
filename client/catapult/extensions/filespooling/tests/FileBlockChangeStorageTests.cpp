/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "tests/test/core/BufferReader.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FileBlockChangeStorageTests

	namespace {
		constexpr auto Empty_Block_Element_Size = sizeof(model::BlockHeader) + sizeof(model::PaddedBlockFooter)
				+ 2 * (Hash256::Size + sizeof(uint32_t));

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

			test::BufferReader reader(buffer);
			EXPECT_EQ(subscribers::BlockChangeOperationType::Block, reader.read<subscribers::BlockChangeOperationType>());

			// - spot check block part of element
			const auto* pBlockData = reader.data();
			EXPECT_EQ(*pBlock, reinterpret_cast<const model::Block&>(*pBlockData));
			reader.advance(Empty_Block_Element_Size);

			// - empty block statement was written
			EXPECT_EQ(0xFFu, reader.read<uint8_t>());

			EXPECT_EQ(0u, reader.read<uint32_t>());
			EXPECT_EQ(0u, reader.read<uint32_t>());
			EXPECT_EQ(0u, reader.read<uint32_t>());
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

			test::BufferReader reader(buffer);
			EXPECT_EQ(subscribers::BlockChangeOperationType::Drop_Blocks_After, reader.read<subscribers::BlockChangeOperationType>());

			EXPECT_EQ(Height(246), reader.read<Height>());
		});
	}
}}
