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
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Helpers for creating a block chain with variable sized blocks.
	struct VariableSizedBlockChain {
		/// Gets the block size at \a height.
		static constexpr uint32_t GetBlockSizeAtHeight(Height height) {
			return SizeOf32<model::BlockHeader>() + static_cast<uint32_t>(height.unwrap() * 100);
		}

		/// Creates storage for a chain with \a numBlocks variable sized blocks.
		static std::unique_ptr<io::BlockStorageCache> CreateStorage(size_t numBlocks) {
			auto pStorage = std::make_unique<io::BlockStorageCache>(
					std::make_unique<mocks::MockMemoryBlockStorage>(),
					std::make_unique<mocks::MockMemoryBlockStorage>());

			// storage already contains nemesis block (height 1)
			auto storageModifier = pStorage->modifier();
			for (auto i = 2u; i <= numBlocks; ++i) {
				auto size = GetBlockSizeAtHeight(Height(i));
				std::vector<uint8_t> buffer(size);
				auto pBlock = reinterpret_cast<model::Block*>(buffer.data());
				pBlock->Size = size;
				pBlock->Height = Height(i);
				pBlock->Difficulty = Difficulty::Min() + Difficulty::Unclamped(1000 + i);
				pBlock->TransactionsPtr()->Size = size - SizeOf32<model::BlockHeader>();
				storageModifier.saveBlock(test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>()));
			}

			storageModifier.commit();
			return pStorage;
		}
	};

	/// Container of height request handler tests.
	template<typename TTraits>
	struct HeightRequestHandlerTests {
	private:
		static std::unique_ptr<io::BlockStorageCache> CreateStorage(size_t numBlocks) {
			return VariableSizedBlockChain::CreateStorage(numBlocks);
		}

	private:
		static void AssertWritesEmptyResponse(size_t numBlocks, Height requestHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			TTraits::Register(handlers, *pStorage);

			auto pPacket = TTraits::CreateRequestPacket();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: only a payload header is written
			test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader), TTraits::ResponsePacketType());
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		}

	public:
		static void AssertDoesNotRespondToMalformedRequest() {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(12);
			TTraits::Register(handlers, *pStorage);

			// - create a malformed request
			auto pPacket = TTraits::CreateRequestPacket();
			pPacket->Height = Height(7);
			++pPacket->Size;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: no response was written because the request was malformed
			test::AssertNoResponse(handlerContext);
		}

		static void AssertWritesEmptyResponseWhenRequestHeightIsLargerThanLocalHeight() {
			// Assert:
			AssertWritesEmptyResponse(12, Height(13));
			AssertWritesEmptyResponse(12, Height(100));
		}

		static void AssertWritesEmptyResponseWhenRequestHeightIsZero() {
			// Assert:
			AssertWritesEmptyResponse(12, Height(0));
		}
	};

#define MAKE_HEIGHT_REQUEST_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::HeightRequestHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_HEIGHT_REQUEST_HANDLER_ALLOW_ZERO_HEIGHT_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_HEIGHT_REQUEST_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, DoesNotRespondToMalformedRequest) \
	MAKE_HEIGHT_REQUEST_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, WritesEmptyResponseWhenRequestHeightIsLargerThanLocalHeight) \

#define DEFINE_HEIGHT_REQUEST_HANDLER_TESTS(TEST_CLASS, HANDLER_NAME) \
	DEFINE_HEIGHT_REQUEST_HANDLER_ALLOW_ZERO_HEIGHT_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_HEIGHT_REQUEST_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, WritesEmptyResponseWhenRequestHeightIsZero)
}}
