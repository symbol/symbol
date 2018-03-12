#include "catapult/handlers/ChainHandlers.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS ChainHandlersTests

	// region PushBlockHandler

	namespace {
		constexpr auto Block_Packet_Size = sizeof(ionet::PacketHeader) + sizeof(model::Block);

		namespace {
			template<typename TAction>
			void RunPushBlockHandlerTest(uint32_t sizeAdjustment, TAction action) {
				// Arrange:
				ionet::ByteBuffer buffer(Block_Packet_Size);
				auto& packet = test::SetPushBlockPacketInBuffer(buffer);
				packet.Size -= sizeAdjustment;

				ionet::ServerPacketHandlers handlers;
				auto registry = mocks::CreateDefaultTransactionRegistry();
				Key capturedSourcePublicKey;
				std::vector<size_t> counters;
				RegisterPushBlockHandler(handlers, registry, [&capturedSourcePublicKey, &counters](const auto& range) {
					capturedSourcePublicKey = range.SourcePublicKey;
					counters.push_back(range.Range.size());
				});

				// Act:
				auto sourcePublicKey = test::GenerateRandomData<Key_Size>();
				ionet::ServerPacketHandlerContext context(sourcePublicKey, "");
				EXPECT_TRUE(handlers.process(packet, context));

				// Assert:
				action(counters);

				// - if the callback was called, context should have been forwarded along with the range
				if (!counters.empty()) {
					EXPECT_EQ(sourcePublicKey, capturedSourcePublicKey);
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

	namespace {
		// use variable-sized blocks in tests
		constexpr uint32_t GetBlockSizeAtHeight(Height height) {
			return static_cast<uint32_t>(sizeof(model::Block) + height.unwrap() * 100u);
		}

		uint32_t GetSumBlockSizesAtHeights(const std::vector<Height>& heights) {
			uint32_t size = 0;
			for (auto height : heights)
				size += GetBlockSizeAtHeight(height);

			return size;
		}

		std::unique_ptr<io::BlockStorageCache> CreateStorage(size_t numBlocks) {
			auto pStorage = std::make_unique<io::BlockStorageCache>(std::make_unique<mocks::MockMemoryBasedStorage>());

			// storage already contains nemesis block (height 1)
			auto storageModifier = pStorage->modifier();
			for (auto i = 2u; i <= numBlocks; ++i) {
				auto size = GetBlockSizeAtHeight(Height(i));
				std::vector<uint8_t> buffer(size);
				auto pBlock = reinterpret_cast<model::Block*>(buffer.data());
				pBlock->Size = size;
				pBlock->Height = Height(i);
				pBlock->Difficulty = Difficulty::Min() + Difficulty::Unclamped(1000 + i);
				reinterpret_cast<model::Transaction*>(pBlock + 1)->Size = size - sizeof(model::Block);
				storageModifier.saveBlock(test::BlockToBlockElement(*pBlock, test::GenerateRandomData<Hash256_Size>()));
			}

			return pStorage;
		}

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

		template<typename TTraits>
		void AssertWritesEmptyResponse(size_t numBlocks, Height requestHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			TTraits::Register(handlers, *pStorage);

			auto pPacket = TTraits::CreateRequestPacket();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: only a payload header is written
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader), TTraits::ResponsePacketType());
			EXPECT_TRUE(context.response().buffers().empty());
		}
	}

// register error handling tests for handlers that require a height
#define HEIGHT_REQUEST_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, PullBlockHandler_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PullBlockHandlerTraits>(); } \
	TEST(TEST_CLASS, BlockHashesHandler_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockHashesHandlerTraits>(); } \
	TEST(TEST_CLASS, PullBlocksHandler_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PullBlocksHandlerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	HEIGHT_REQUEST_TEST(DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		TTraits::Register(handlers, *pStorage);

		// - create a malformed request
		auto pPacket = TTraits::CreateRequestPacket();
		pPacket->Height = Height(7);
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext context({}, "");
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert: no response was written because the request was malformed
		test::AssertNoResponse(context);
	}

	HEIGHT_REQUEST_TEST(WritesEmptyResponseIfRequestHeightIsLargerThanLocalHeight) {
		// Assert:
		AssertWritesEmptyResponse<TTraits>(12, Height(13));
		AssertWritesEmptyResponse<TTraits>(12, Height(100));
	}

	// region PullBlockHandler

	namespace {
		void AssertCanRetrieveBlock(size_t numBlocks, Height requestHeight, Height expectedHeight) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			RegisterPullBlockHandler(handlers, *pStorage);

			auto pPacket = ionet::CreateSharedPacket<api::PullBlockRequest>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + GetBlockSizeAtHeight(expectedHeight);
			test::AssertPacketHeader(context, expectedSize, ionet::PacketType::Pull_Block);

			auto pBlockFromStorage = pStorage->view().loadBlock(expectedHeight);
			EXPECT_EQ(*pBlockFromStorage, reinterpret_cast<const model::Block&>(*test::GetSingleBufferData(context)));
		}
	}

	TEST(TEST_CLASS, PullBlockHandler_WritesBlockAtHeightToResponse) {
		// Assert:
		AssertCanRetrieveBlock(12, Height(7), Height(7));
	}

	TEST(TEST_CLASS, PullBlockHandler_CanExplicitlyRetrieveLastBlockAtHeight) {
		// Assert:
		AssertCanRetrieveBlock(12, Height(12), Height(12));
	}

	TEST(TEST_CLASS, PullBlockHandler_CanImplicitlyRetrieveLastBlock) {
		// Assert:
		AssertCanRetrieveBlock(12, Height(0), Height(12));
	}

	// endregion

	// region ChainInfoHandler

	TEST(TEST_CLASS, ChainInfoHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		RegisterChainInfoHandler(
				handlers,
				*pStorage,
				[]() { return model::ChainScore(0x7890ABCDEF012345, 0x7711BBCC00DD99AA); });

		// - malform the packet
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Chain_Info;
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext context({}, "");
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert: malformed packet is ignored
		test::AssertNoResponse(context);
	}

	TEST(TEST_CLASS, ChainInfoHandler_WritesChainInfoResponseInResponseToValidRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		auto pStorage = CreateStorage(12);
		RegisterChainInfoHandler(
				handlers,
				*pStorage,
				[]() { return model::ChainScore(0x7890ABCDEF012345, 0x7711BBCC00DD99AA); });

		// - create a valid request
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Chain_Info;

		// Act:
		ionet::ServerPacketHandlerContext context({}, "");
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert: chain score is written
		test::AssertPacketHeader(context, sizeof(api::ChainInfoResponse), ionet::PacketType::Chain_Info);

		const auto* pResponse = reinterpret_cast<const uint64_t*>(test::GetSingleBufferData(context));
		EXPECT_EQ(12u, pResponse[0]); // height
		EXPECT_EQ(0x7890ABCDEF012345, pResponse[1]); // score high
		EXPECT_EQ(0x7711BBCC00DD99AA, pResponse[2]); // score low
	}

	// endregion

	// region BlockHashesHandler

	TEST(TEST_CLASS, BlockHashesHandler_WritesEmptyResponseIfRequestHeightIsZero) {
		// Assert:
		AssertWritesEmptyResponse<BlockHashesHandlerTraits>(12, Height(0));
	}

	namespace {
		void AssertCanRetrieveHashes(
				size_t numBlocks,
				uint32_t maxHashes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			RegisterBlockHashesHandler(handlers, *pStorage, maxHashes);

			auto pPacket = ionet::CreateSharedPacket<api::BlockHashesRequest>();
			pPacket->Height = requestHeight;

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + sizeof(Hash256) * expectedHeights.size();
			test::AssertPacketHeader(context, expectedSize, ionet::PacketType::Block_Hashes);

			auto pData = test::GetSingleBufferData(context);
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

	TEST(TEST_CLASS, BlockHashesHandler_WritesAtMostMaxHashes) {
		// Assert:
		AssertCanRetrieveHashes(12, 5, Height(3), { Height(3), Height(4), Height(5), Height(6), Height(7) });
	}

	TEST(TEST_CLASS, BlockHashesHandler_WritesAreBoundedByLastBlock) {
		// Assert:
		AssertCanRetrieveHashes(12, 10, Height(10), { Height(10), Height(11), Height(12) });
	}

	TEST(TEST_CLASS, BlockHashesHandler_CanRetrieveLastBlockHash) {
		// Assert:
		AssertCanRetrieveHashes(12, 5, Height(12), { Height(12) });
	}

	// endregion

	// region BlocksHandler

	TEST(TEST_CLASS, PullBlocksHandler_WritesEmptyResponseIfRequestHeightIsZero) {
		// Assert:
		AssertWritesEmptyResponse<PullBlocksHandlerTraits>(12, Height(0));
	}

	namespace {
		void AssertCanRetrieveBlocks(
				size_t numBlocks,
				const api::PullBlocksRequest& request,
				const PullBlocksHandlerConfiguration& config,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pStorage = CreateStorage(numBlocks);
			RegisterPullBlocksHandler(handlers, *pStorage, config);

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(request, context));

			// Assert:
			auto expectedSize = sizeof(ionet::PacketHeader) + GetSumBlockSizesAtHeights(expectedHeights);
			test::AssertPacketHeader(context, expectedSize, ionet::PacketType::Pull_Blocks);

			const auto& buffers = context.response().buffers();
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
				size_t numBlocks,
				uint32_t maxBlocks,
				uint32_t maxResponseBytes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange: set request NumXyz == config MaxXyz
			auto pRequest = ionet::CreateSharedPacket<api::PullBlocksRequest>();
			pRequest->Height = requestHeight;
			pRequest->NumBlocks = maxBlocks;
			pRequest->NumResponseBytes = maxResponseBytes;

			PullBlocksHandlerConfiguration config;
			config.MaxBlocks = maxBlocks;
			config.MaxResponseBytes = maxResponseBytes;

			// Assert:
			AssertCanRetrieveBlocks(numBlocks, *pRequest, config, expectedHeights);
		}
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtMostMaxBlocks) {
		// Assert:
		std::vector<Height> expectedBlockHeights{ Height(3), Height(4), Height(5), Height(6), Height(7) };
		AssertCanRetrieveBlocks(12, 5, 10 * 1024 * 1024, Height(3), expectedBlockHeights);
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtMostMaxResponseBytes) {
		// Arrange:
		auto assertFunc = [](int32_t delta, const std::vector<Height>& expectedBlockHeights) {
			// - calculate the sum of the sizes of blocks 3-5
			auto responseBytes = GetSumBlockSizesAtHeights({ Height(3), Height(4), Height(5) });
			responseBytes += static_cast<uint32_t>(delta);
			AssertCanRetrieveBlocks(12, 5, responseBytes, Height(3), expectedBlockHeights);
		};

		// Assert: only blocks that fully fit within the requested size are returned
		assertFunc(-1, { Height(3), Height(4) });
		assertFunc(0, { Height(3), Height(4), Height(5) });
		assertFunc(1, { Height(3), Height(4), Height(5) });
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAtLeastOneBlock) {
		// Assert: even with num/max response bytes set to 0, the first block should be returned
		AssertCanRetrieveBlocks(12, 5, 0, Height(3), { Height(3) });
	}

	TEST(TEST_CLASS, PullBlocksHandler_WritesAreBoundedByLastBlock) {
		// Assert:
		AssertCanRetrieveBlocks(12, 10, 10 * 1024 * 1024, Height(10), { Height(10), Height(11), Height(12) });
	}

	TEST(TEST_CLASS, PullBlocksHandler_CanRetrieveLastBlock) {
		// Assert:
		AssertCanRetrieveBlocks(12, 5, 10 * 1024 * 1024, Height(12), { Height(12) });
	}

	namespace {
		void AssertCanRetrieveBlocksWithNumBlocksClamping(
				size_t numBlocks,
				uint32_t numRequestBlocks,
				uint32_t maxBlocks,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			auto pRequest = ionet::CreateSharedPacket<api::PullBlocksRequest>();
			pRequest->Height = requestHeight;
			pRequest->NumBlocks = numRequestBlocks;
			pRequest->NumResponseBytes = 100 * 1024 * 1024;

			PullBlocksHandlerConfiguration config;
			config.MaxBlocks = maxBlocks;
			config.MaxResponseBytes = 100 * 1024 * 1024;

			// Assert:
			AssertCanRetrieveBlocks(numBlocks, *pRequest, config, expectedHeights);
		}

		TEST(TEST_CLASS, PullBlocksHandler_NumBlocksIsClampedByMaxBlocks) {
			// Arrange: chain-size == 12, request-height == 2, max == 7
			auto assertFunc = [](auto numRequestBlocks, const auto& expectedBlockHeights) {
				AssertCanRetrieveBlocksWithNumBlocksClamping(12, numRequestBlocks, 7, Height(2), expectedBlockHeights);
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
				size_t numBlocks,
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
			AssertCanRetrieveBlocks(numBlocks, *pRequest, config, expectedHeights);
		}

		TEST(TEST_CLASS, PullBlocksHandler_NumResponseBytesIsClampedByMaxResponseBytes) {
			// Arrange: chain-size == 12, request-height == 2, max == 7
			std::vector<Height> heights{ Height(2), Height(3), Height(4), Height(5), Height(6), Height(7), Height(8) };
			auto maxBytes = GetSumBlockSizesAtHeights(heights);
			auto assertFunc = [maxBytes](auto numRequestResponseBytes, const auto& expectedBlockHeights) {
				AssertCanRetrieveBlocksWithNumResponseBytesClamping(
						12,
						numRequestResponseBytes,
						maxBytes,
						Height(2),
						expectedBlockHeights);
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
