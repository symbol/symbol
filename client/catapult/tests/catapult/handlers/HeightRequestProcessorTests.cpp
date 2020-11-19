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

#include "catapult/handlers/HeightRequestProcessor.h"
#include "catapult/api/ChainPackets.h"
#include "tests/catapult/handlers/test/HeightRequestHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS HeightRequestProcessorTests

	namespace {
		constexpr auto CreateStorage = test::VariableSizedBlockChain::CreateStorage;

		using HeightRequestPacket = api::HeightPacket<static_cast<ionet::PacketType>(1234)>;
		using Processor = HeightRequestProcessor<HeightRequestPacket>;
	}

	TEST(TEST_CLASS, AbortsProcessingWhenRequestIsMalformed) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);

		// - create a malformed request
		auto pPacket = ionet::CreateSharedPacket<HeightRequestPacket>();
		pPacket->Height = Height(7);
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		auto heightRequestInfo = Processor::Process(pStorage->view(), *pPacket, handlerContext, false);

		// Assert: empty info was returned
		EXPECT_FALSE(!!heightRequestInfo.pRequest);

		// - no response was written because the request was malformed
		test::AssertNoResponse(handlerContext);
	}

	namespace {
		void AssertWritesEmptyResponse(size_t numBlocks, Height requestHeight, bool shouldAllowZeroHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);

			auto pPacket = ionet::CreateSharedPacket<HeightRequestPacket>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			auto heightRequestInfo = Processor::Process(pStorage->view(), *pPacket, handlerContext, shouldAllowZeroHeight);

			// Assert: empty info was returned
			EXPECT_FALSE(!!heightRequestInfo.pRequest);

			// Assert: only a payload header is written
			test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader), HeightRequestPacket::Packet_Type);
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		}
	}

	TEST(TEST_CLASS, AbortsProcessingWhenRequestHeightIsLargerThanLocalHeight) {
		AssertWritesEmptyResponse(12, Height(13), false);
		AssertWritesEmptyResponse(12, Height(100), false);
	}

	TEST(TEST_CLASS, AbortsProcessingWhenRequestHeightIsZeroAndZeroIsDisallowed) {
		AssertWritesEmptyResponse(12, Height(0), false);
	}

	namespace {
		template<typename TAssertInfo>
		auto AssertAllowsProcessingOfValidHeight(
				size_t numBlocks,
				Height requestHeight,
				bool shouldAllowZeroHeight,
				TAssertInfo assertInfo) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);

			auto pPacket = ionet::CreateSharedPacket<HeightRequestPacket>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			auto heightRequestInfo = Processor::Process(pStorage->view(), *pPacket, handlerContext, shouldAllowZeroHeight);

			// Assert: no response was written because the request processing is incomplete
			test::AssertNoResponse(handlerContext);
			assertInfo(heightRequestInfo);
		}
	}

	TEST(TEST_CLASS, AllowsProcessingWhenRequestHeightIsLessThanChainHeight) {
		AssertAllowsProcessingOfValidHeight(12, Height(7), false, [](const auto& heightRequestInfo) {
			ASSERT_TRUE(!!heightRequestInfo.pRequest);
			EXPECT_EQ(Height(7), heightRequestInfo.pRequest->Height);

			EXPECT_EQ(Height(12), heightRequestInfo.ChainHeight);
			EXPECT_EQ(Height(7), heightRequestInfo.NormalizedRequestHeight);
			EXPECT_EQ(6u, heightRequestInfo.numAvailableBlocks());
		});
	}

	TEST(TEST_CLASS, AllowsProcessingWhenRequestHeightIsEqualToChainHeight) {
		AssertAllowsProcessingOfValidHeight(12, Height(12), false, [](const auto& heightRequestInfo) {
			ASSERT_TRUE(!!heightRequestInfo.pRequest);
			EXPECT_EQ(Height(12), heightRequestInfo.pRequest->Height);

			EXPECT_EQ(Height(12), heightRequestInfo.ChainHeight);
			EXPECT_EQ(Height(12), heightRequestInfo.NormalizedRequestHeight);
			EXPECT_EQ(1u, heightRequestInfo.numAvailableBlocks());
		});
	}

	TEST(TEST_CLASS, AllowsProcessingWhenRequestHeightIsZeroAndZeroIsAllowed) {
		AssertAllowsProcessingOfValidHeight(12, Height(0), true, [](const auto& heightRequestInfo) {
			ASSERT_TRUE(!!heightRequestInfo.pRequest);
			EXPECT_EQ(Height(0), heightRequestInfo.pRequest->Height);

			EXPECT_EQ(Height(12), heightRequestInfo.ChainHeight);
			EXPECT_EQ(Height(12), heightRequestInfo.NormalizedRequestHeight);
			EXPECT_EQ(1u, heightRequestInfo.numAvailableBlocks());
		});
	}
}}
