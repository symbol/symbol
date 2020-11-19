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

#include "catapult/subscribers/BlockChangeReader.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS BlockChangeReaderTests

	namespace {
		constexpr auto Empty_Block_Element_Size = sizeof(model::BlockHeader) + sizeof(model::PaddedBlockFooter)
				+ 2 * (Hash256::Size + sizeof(uint32_t));

		std::vector<uint8_t> CreateSerializedDataBuffer(BlockChangeOperationType operationType, Height height) {
			std::vector<uint8_t> buffer(1 + sizeof(Height));
			buffer[0] = utils::to_underlying_type(operationType);
			std::memcpy(&buffer[1], &height, sizeof(Height));
			return buffer;
		}

		std::vector<uint8_t> CreateSerializedDataBuffer(const model::Block& block, bool includeStatements) {
			std::vector<uint8_t> buffer(1 + Empty_Block_Element_Size + 1 + (includeStatements ? 3 * sizeof(uint32_t) : 0), 0);
			buffer[0] = utils::to_underlying_type(BlockChangeOperationType::Block);
			std::memcpy(&buffer[1], &block, block.Size);
			buffer[1 + Empty_Block_Element_Size] = includeStatements ? 0xFF : 0;
			return buffer;
		}

		void AssertCanReadSingleBlockChange(bool includeStatements) {
			// Arrange:
			auto pBlock = test::GenerateEmptyRandomBlock();
			auto buffer = CreateSerializedDataBuffer(*pBlock, includeStatements);

			mocks::MockMemoryStream stream(buffer);
			mocks::MockBlockChangeSubscriber subscriber;

			// Act:
			ReadNextBlockChange(stream, subscriber);

			// Assert:
			ASSERT_EQ(1u, subscriber.blockElements().size());
			EXPECT_EQ(0u, subscriber.dropBlocksAfterHeights().size());

			const auto& readBlockElement = *subscriber.copiedBlockElements()[0];
			EXPECT_EQ(*pBlock, readBlockElement.Block);
			EXPECT_EQ(includeStatements, !!readBlockElement.OptionalStatement);
		}
	}

	TEST(TEST_CLASS, CanReadSingleBlockChange_BlockWithoutStatements) {
		AssertCanReadSingleBlockChange(false);
	}

	TEST(TEST_CLASS, CanReadSingleBlockChange_BlockWithStatements) {
		AssertCanReadSingleBlockChange(true);
	}

	TEST(TEST_CLASS, CanReadSingleDropBlocksAfterChange) {
		// Arrange:
		auto buffer = CreateSerializedDataBuffer(BlockChangeOperationType::Drop_Blocks_After, Height(987));

		mocks::MockMemoryStream stream(buffer);
		mocks::MockBlockChangeSubscriber subscriber;

		// Act:
		ReadNextBlockChange(stream, subscriber);

		// Assert:
		EXPECT_EQ(0u, subscriber.blockElements().size());
		ASSERT_EQ(1u, subscriber.dropBlocksAfterHeights().size());

		EXPECT_EQ(Height(987), subscriber.dropBlocksAfterHeights()[0]);
	}

	TEST(TEST_CLASS, CannotReadSingleUnknownOperationType) {
		// Arrange:
		auto buffer = CreateSerializedDataBuffer(static_cast<BlockChangeOperationType>(123), Height(987));

		mocks::MockMemoryStream stream(buffer);
		mocks::MockBlockChangeSubscriber subscriber;

		// Act + Assert:
		EXPECT_THROW(ReadNextBlockChange(stream, subscriber), catapult_invalid_argument);
	}
}}
