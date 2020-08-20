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

#include "catapult/handlers/ChainHandlers.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/utils/FileSize.h"
#include "tests/catapult/handlers/test/HeightRequestHandlerTests.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS ChainHandlersTests

	namespace {
		constexpr auto GetBlockSizeAtHeight = test::VariableSizedBlockChain::GetBlockSizeAtHeight;
		constexpr auto CreateStorage = test::VariableSizedBlockChain::CreateStorage;

		uint32_t GetSumBlockSizesAtHeights(const std::vector<Height>& heights) {
			uint32_t size = 0;
			for (auto height : heights)
				size += GetBlockSizeAtHeight(height);

			return size;
		}
	}

	// region PushBlockHandler

	namespace {
		constexpr auto Block_Packet_Size = sizeof(ionet::PacketHeader) + sizeof(model::BlockHeader);

		namespace {
			template<typename TAction>
			void RunPushBlockHandlerTest(uint32_t sizeAdjustment, TAction action) {
				// Arrange:
				ionet::ByteBuffer buffer(Block_Packet_Size);
				auto& packet = test::SetPushBlockPacketInBuffer(buffer);
				packet.Size -= sizeAdjustment;

				ionet::ServerPacketHandlers handlers;
				auto registry = mocks::CreateDefaultTransactionRegistry();
				model::NodeIdentity capturedSourceIdentity;
				std::vector<size_t> counters;
				RegisterPushBlockHandler(handlers, registry, [&capturedSourceIdentity, &counters](const auto& range) {
					capturedSourceIdentity = range.SourceIdentity;
					counters.push_back(range.Range.size());
				});

				// Act:
				auto sourcePublicKey = test::GenerateRandomByteArray<Key>();
				auto sourceHost = std::string("11.22.33.44");
				ionet::ServerPacketHandlerContext handlerContext(sourcePublicKey, sourceHost);
				EXPECT_TRUE(handlers.process(packet, handlerContext));

				// Assert:
				action(counters);

				// - if the callback was called, handlerContext should have been forwarded along with the range
				if (!counters.empty()) {
					EXPECT_EQ(sourcePublicKey, capturedSourceIdentity.PublicKey);
					EXPECT_EQ("11.22.33.44", capturedSourceIdentity.Host);
				}
			}
		}
	}

	TEST(TEST_CLASS, PushBlockHandler_MalformedBlockIsNotForwardedToDisruptor) {
		// Arrange:
		RunPushBlockHandlerTest(1, [](const auto& counters) {
			// Assert:
			EXPECT_TRUE(counters.empty());
		});
	}

	TEST(TEST_CLASS, PushBlockHandler_WellFormedBlockIsForwardedToDisruptor) {
		// Arrange:
		RunPushBlockHandlerTest(0, [](const auto& counters) {
			// Assert:
			ASSERT_EQ(1u, counters.size());
			EXPECT_EQ(1u, counters[0]);
		});
	}

	// endregion

	// region PullBlockHandler

	namespace {
		struct PullBlockHandlerTraits {
			static ionet::PacketType ResponsePacketType() {
				return ionet::PacketType::Pull_Block;
			}

			static auto CreateRequestPacket() {
				return ionet::CreateSharedPacket<api::PullBlockRequest>();
			}

			static void Register(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
				RegisterPullBlockHandler(handlers, storage);
			}
		};
	}

	DEFINE_HEIGHT_REQUEST_HANDLER_ALLOW_ZERO_HEIGHT_TESTS(PullBlockHandlerTraits, PullBlockHandler)

	namespace {
		void AssertCanRetrieveBlock(size_t numBlocks, Height requestHeight, Height expectedHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			RegisterPullBlockHandler(handlers, *pStorage);

			auto pPacket = ionet::CreateSharedPacket<api::PullBlockRequest>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + GetBlockSizeAtHeight(expectedHeight);
			test::AssertPacketHeader(handlerContext, expectedSize, ionet::PacketType::Pull_Block);

			auto pBlockFromStorage = pStorage->view().loadBlock(expectedHeight);
			EXPECT_EQ(*pBlockFromStorage, reinterpret_cast<const model::Block&>(*test::GetSingleBufferData(handlerContext)));
		}
	}

	TEST(TEST_CLASS, PullBlockHandler_WritesBlockAtHeightToResponse) {
		AssertCanRetrieveBlock(12, Height(7), Height(7));
	}

	TEST(TEST_CLASS, PullBlockHandler_CanExplicitlyRetrieveLastBlockAtHeight) {
		AssertCanRetrieveBlock(12, Height(12), Height(12));
	}

	TEST(TEST_CLASS, PullBlockHandler_CanImplicitlyRetrieveLastBlock) {
		AssertCanRetrieveBlock(12, Height(0), Height(12));
	}

	// endregion

	// region ChainStatisticsHandler

	TEST(TEST_CLASS, ChainStatisticsHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		RegisterChainStatisticsHandler(
				handlers,
				*pStorage,
				[]() { return model::ChainScore(0x7890ABCDEF012345, 0x7711BBCC00DD99AA); },
				[]() { return Height(7); });

		// - malform the packet
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Chain_Statistics;
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: malformed packet is ignored
		test::AssertNoResponse(handlerContext);
	}

	TEST(TEST_CLASS, ChainStatisticsHandler_WritesChainStatisticsResponseInResponseToValidRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		RegisterChainStatisticsHandler(
				handlers,
				*pStorage,
				[]() { return model::ChainScore(0x7890ABCDEF012345, 0x7711BBCC00DD99AA); },
				[]() { return Height(7); });

		// - create a valid request
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Chain_Statistics;

		// Act:
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

		// Assert: chain statistics are written
		test::AssertPacketHeader(handlerContext, sizeof(api::ChainStatisticsResponse), ionet::PacketType::Chain_Statistics);

		const auto* pResponse = reinterpret_cast<const uint64_t*>(test::GetSingleBufferData(handlerContext));
		EXPECT_EQ(12u, pResponse[0]); // height
		EXPECT_EQ(7u, pResponse[1]); // finalized height
		EXPECT_EQ(0x7890ABCDEF012345u, pResponse[2]); // score high
		EXPECT_EQ(0x7711BBCC00DD99AAu, pResponse[3]); // score low
	}

	// endregion

	// region BlockHashesHandler

	namespace {
		struct BlockHashesHandlerTraits {
			static ionet::PacketType ResponsePacketType() {
				return ionet::PacketType::Block_Hashes;
			}

			static auto CreateRequestPacket() {
				return ionet::CreateSharedPacket<api::BlockHashesRequest>();
			}

			static void Register(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
				RegisterBlockHashesHandler(handlers, storage, 10);
			}
		};
	}

	DEFINE_HEIGHT_REQUEST_HANDLER_TESTS(BlockHashesHandlerTraits, BlockHashesHandler)

	namespace {
		void AssertCanRetrieveHashes(uint32_t maxRequestedHashes, Height requestHeight, const std::vector<Height>& expectedHeights) {
			// Arrange: 12 blocks, on remote side allow at most 7 hashes
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(12);
			RegisterBlockHashesHandler(handlers, *pStorage, 7);

			auto pPacket = ionet::CreateSharedPacket<api::BlockHashesRequest>();
			pPacket->Height = requestHeight;
			pPacket->NumHashes = maxRequestedHashes;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + sizeof(Hash256) * expectedHeights.size();
			test::AssertPacketHeader(handlerContext, expectedSize, ionet::PacketType::Block_Hashes);

			const auto* pData = test::GetSingleBufferData(handlerContext);
			auto storageView = pStorage->view();
			for (auto i = 0u; i < expectedHeights.size(); ++i) {
				// - calculate the expected hash
				auto expectedHash = storageView.loadBlockElement(expectedHeights[i])->EntityHash;

				// - retrieve the actual hash
				const auto& hash = reinterpret_cast<const Hash256&>(*pData);

				// - compare
				EXPECT_EQ(expectedHash, hash) << "comparing hashes at " << i << " from " << requestHeight;
				pData += sizeof(Hash256);
			}
		}
	}

	TEST(TEST_CLASS, BlockHashesHandler_WritesAtMostMaxHashesOnRemote) {
		AssertCanRetrieveHashes(100, Height(3), { Height(3), Height(4), Height(5), Height(6), Height(7), Height(8), Height(9) });
	}

	TEST(TEST_CLASS, BlockHashesHandler_WritesAtMostMaxRequestedHashes) {
		AssertCanRetrieveHashes(5, Height(3), { Height(3), Height(4), Height(5), Height(6), Height(7) });
	}

	TEST(TEST_CLASS, BlockHashesHandler_WritesAreBoundedByLastBlock) {
		AssertCanRetrieveHashes(5, Height(10), { Height(10), Height(11), Height(12) });
	}

	TEST(TEST_CLASS, BlockHashesHandler_CanRetrieveLastBlockHash) {
		AssertCanRetrieveHashes(5, Height(12), { Height(12) });
	}

	// endregion

	// region PullBlocksHandler

	namespace {
		struct PullBlocksHandlerTraits {
			static ionet::PacketType ResponsePacketType() {
				return ionet::PacketType::Pull_Blocks;
			}

			static auto CreateRequestPacket() {
				auto pRequest = ionet::CreateSharedPacket<api::PullBlocksRequest>();
				pRequest->NumBlocks = 100;
				pRequest->NumResponseBytes = 10 * 1024 * 1024;
				return pRequest;
			}

			static void Register(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage) {
				PullBlocksHandlerConfiguration config;
				config.MaxBlocks = 100;
				config.MaxResponseBytes = 10 * 1024 * 1024;
				RegisterPullBlocksHandler(handlers, storage, config);
			}
		};
	}

	DEFINE_HEIGHT_REQUEST_HANDLER_TESTS(PullBlocksHandlerTraits, PullBlocksHandler)

	namespace {
		constexpr auto Ten_Megabytes = static_cast<uint32_t>(utils::FileSize::FromMegabytes(10).bytes());

		void AssertCanRetrieveBlocks(
				const api::PullBlocksRequest& request,
				const PullBlocksHandlerConfiguration& config,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(12);
			RegisterPullBlocksHandler(handlers, *pStorage, config);

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(request, handlerContext));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + GetSumBlockSizesAtHeights(expectedHeights);
			test::AssertPacketHeader(handlerContext, expectedSize, ionet::PacketType::Pull_Blocks);

			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(expectedHeights.size(), buffers.size());
			auto storageView = pStorage->view();
			for (auto i = 0u; i < expectedHeights.size(); ++i) {
				// - load the expected block
				auto pBlockFromStorage = storageView.loadBlock(expectedHeights[i]);

				// - retrieve the actual block
				const auto& block = reinterpret_cast<const model::Block&>(*buffers[i].pData);

				// - compare
				EXPECT_EQ(*pBlockFromStorage, block)
						<< "comparing blocks at " << i << " from " << request.Height;
			}
		}

		void AssertCanRetrieveBlocks(
				uint32_t numRequestBlocks,
				uint32_t maxBlocks,
				uint32_t numRequestResponseBytes,
				uint32_t maxResponseBytes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange: set request NumXyz == config MaxXyz
			auto pRequest = PullBlocksHandlerTraits::CreateRequestPacket();
			pRequest->Height = requestHeight;
			pRequest->NumBlocks = numRequestBlocks;
			pRequest->NumResponseBytes = numRequestResponseBytes;

			PullBlocksHandlerConfiguration config;
			config.MaxBlocks = maxBlocks;
			config.MaxResponseBytes = maxResponseBytes;

			// Assert:
			AssertCanRetrieveBlocks(*pRequest, config, expectedHeights);
		}

		void AssertCanRetrieveBlocks(
				uint32_t maxBlocks,
				uint32_t maxResponseBytes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			AssertCanRetrieveBlocks(maxBlocks, maxBlocks, maxResponseBytes, maxResponseBytes, requestHeight, expectedHeights);
		}

		void AssertWritesAtMostMaxBytes(const consumer<uint32_t, const std::vector<Height>&>& assertFunc) {
			// Arrange: calculate the sum of the sizes of blocks 3-5
			auto responseBytes = GetSumBlockSizesAtHeights({ Height(3), Height(4), Height(5) });
			auto adjustResponseBytes = [responseBytes](int32_t delta) { return responseBytes + static_cast<uint32_t>(delta); };

			// Assert: only blocks that fully fit within the requested size are returned
			assertFunc(adjustResponseBytes(-1), { Height(3), Height(4) });
			assertFunc(adjustResponseBytes(0), { Height(3), Height(4), Height(5) });
			assertFunc(adjustResponseBytes(1), { Height(3), Height(4), Height(5) });
		}
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtMostMaxBlocks) {
		std::vector<Height> expectedBlockHeights{ Height(3), Height(4), Height(5), Height(6), Height(7) };
		AssertCanRetrieveBlocks(10, 5, Ten_Megabytes, Ten_Megabytes, Height(3), expectedBlockHeights);
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtMostMaxResponseBytes) {
		AssertWritesAtMostMaxBytes([](auto responseBytes, const auto& expectedBlockHeights) {
			AssertCanRetrieveBlocks(5, 5, Ten_Megabytes, responseBytes, Height(3), expectedBlockHeights);
		});
	}

	TEST(TEST_CLASS, PullBlocksHandler_RespectsRequestNumBlocks) {
		std::vector<Height> expectedBlockHeights{ Height(3), Height(4), Height(5) };
		AssertCanRetrieveBlocks(3, 5, Ten_Megabytes, Ten_Megabytes, Height(3), expectedBlockHeights);
	}

	TEST(TEST_CLASS, PullBlocksHandler_RespectsRequestMaxResponseBytes) {
		AssertWritesAtMostMaxBytes([](auto responseBytes, const auto& expectedBlockHeights) {
			AssertCanRetrieveBlocks(5, 5, responseBytes, Ten_Megabytes, Height(3), expectedBlockHeights);
		});
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtLeastOneBlock) {
		// Assert: even with num/max response bytes set to 0, the first block should be returned
		AssertCanRetrieveBlocks(5, 0, Height(3), { Height(3) });
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAreBoundedByLastBlock) {
		AssertCanRetrieveBlocks(10, Ten_Megabytes, Height(10), { Height(10), Height(11), Height(12) });
	}

	TEST(TEST_CLASS, PullBlocksHandler_CanRetrieveLastBlock) {
		AssertCanRetrieveBlocks(5, Ten_Megabytes, Height(12), { Height(12) });
	}

	namespace {
		void AssertCanRetrieveBlocksWithNumBlocksClamping(
				uint32_t numRequestBlocks,
				uint32_t maxBlocks,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			auto pRequest = ionet::CreateSharedPacket<api::PullBlocksRequest>();
			pRequest->Height = requestHeight;
			pRequest->NumBlocks = numRequestBlocks;
			pRequest->NumResponseBytes = 10 * Ten_Megabytes;

			PullBlocksHandlerConfiguration config;
			config.MaxBlocks = maxBlocks;
			config.MaxResponseBytes = 10 * Ten_Megabytes;

			// Assert:
			AssertCanRetrieveBlocks(*pRequest, config, expectedHeights);
		}

		TEST(TEST_CLASS, PullBlocksHandler_NumBlocksIsClampedByMaxBlocks) {
			// Arrange: chain-size == 12, request-height == 2, max == 7
			auto assertFunc = [](auto numRequestBlocks, const auto& expectedBlockHeights) {
				AssertCanRetrieveBlocksWithNumBlocksClamping(numRequestBlocks, 7, Height(2), expectedBlockHeights);
			};

			// Assert:
			std::vector<Height> expectedBlockHeights{ Height(2), Height(3), Height(4), Height(5), Height(6) };

			// - request (5 < max) is unchanged
			assertFunc(5u, expectedBlockHeights);

			// - request (9 > max) is decreased to max (7)
			expectedBlockHeights.push_back(Height(7));
			expectedBlockHeights.push_back(Height(8));
			assertFunc(9u, expectedBlockHeights);
		}

		void AssertCanRetrieveBlocksWithNumResponseBytesClamping(
				uint32_t numRequestResponseBytes,
				uint32_t maxResponseBytes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			auto pRequest = ionet::CreateSharedPacket<api::PullBlocksRequest>();
			pRequest->Height = requestHeight;
			pRequest->NumBlocks = 100;
			pRequest->NumResponseBytes = numRequestResponseBytes;

			PullBlocksHandlerConfiguration config;
			config.MaxBlocks = 100;
			config.MaxResponseBytes = maxResponseBytes;

			// Assert:
			AssertCanRetrieveBlocks(*pRequest, config, expectedHeights);
		}

		TEST(TEST_CLASS, PullBlocksHandler_NumResponseBytesIsClampedByMaxResponseBytes) {
			// Arrange: chain-size == 12, request-height == 2, max == 7
			std::vector<Height> heights{ Height(2), Height(3), Height(4), Height(5), Height(6), Height(7), Height(8) };
			auto maxBytes = GetSumBlockSizesAtHeights(heights);
			auto assertFunc = [maxBytes](auto numRequestResponseBytes, const auto& expectedBlockHeights) {
				AssertCanRetrieveBlocksWithNumResponseBytesClamping(numRequestResponseBytes, maxBytes, Height(2), expectedBlockHeights);
			};

			// Assert:
			std::vector<Height> expectedBlockHeights{ Height(2), Height(3), Height(4), Height(5), Height(6) };

			// - requestBytes < maxBytes is unchanged
			auto requestBytes = GetSumBlockSizesAtHeights(expectedBlockHeights);
			assertFunc(requestBytes, expectedBlockHeights);

			// - requestBytes > maxBytes is decreased to maxBytes
			expectedBlockHeights.push_back(Height(7));
			expectedBlockHeights.push_back(Height(8));
			requestBytes = maxBytes + GetSumBlockSizesAtHeights({ Height(9), Height(10) });
			assertFunc(requestBytes, expectedBlockHeights);
		}
	}

	// endregion
}}
