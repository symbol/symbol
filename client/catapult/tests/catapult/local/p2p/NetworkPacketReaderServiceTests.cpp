#include "catapult/local/p2p/NetworkPacketReaderService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/handlers/HandlerFactory.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		using ServiceType = NetworkPacketReaderService;

		namespace {
			struct SquaresTraits {
				using EntityType = uint64_t;
				using SupplierResultsType = std::vector<uint64_t>;

				static constexpr auto Packet_Type = static_cast<ionet::PacketType>(25);

				static auto ToPayload(const SupplierResultsType& results) {
					auto payloadSize = utils::checked_cast<size_t, uint32_t>(results.size() * sizeof(uint64_t));
					auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
					pPacket->Type = Packet_Type;
					std::memcpy(pPacket->Data(), results.data(), payloadSize);
					return pPacket;
				}
			};

			std::unique_ptr<plugins::PluginManager> CreatePluginManager() {
				// Arrange:
				auto pPluginManager = std::make_unique<plugins::PluginManager>(model::BlockChainConfiguration::Uninitialized());

				// - add support for mock transactions
				pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin());

				// - add a dummy diagnostic api
				pPluginManager->addDiagnosticHandlerHook([](auto& handlers, const auto&) {
					using HandlerFactory = handlers::BatchHandlerFactory<SquaresTraits>;
					handlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create([](const auto& range) {
						std::vector<uint64_t> squares;
						for (const auto& value : range)
							squares.push_back(value * value);

						return squares;
					}));
				});

				return pPluginManager;
			}
		}

		class TestContext {
		public:
			TestContext(Height networkHeight = Height())
					: m_keyPair(test::GenerateKeyPair())
					, m_state(test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized()))
					, m_pPluginManager(CreatePluginManager())
					, m_pUnconfirmedTransactionsCache(test::CreateUnconfirmedTransactionsCache())
					, m_networkHeight(networkHeight.unwrap())
					, m_numConsumedBlockElements(0)
					, m_numConsumedTransactionElements(0)
					, m_service(
							m_keyPair,
							m_state.cref(),
							*m_pPluginManager,
							*m_pUnconfirmedTransactionsCache,
							m_networkHeight,
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service consumed block element";
								++m_numConsumedBlockElements;
							},
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service consumed transaction element";
								++m_numConsumedTransactionElements;
							})
					, m_pool(2, "reader")
			{}

		public:
			auto& pool() {
				return m_pool;
			}

			auto& service() {
				return m_service;
			}

			void boot() {
				m_service.boot(m_pool);
			}

		public:
			size_t numConsumedBlockElements() const {
				return m_numConsumedBlockElements;
			}

			size_t numConsumedTransactionElements() const {
				return m_numConsumedTransactionElements;
			}

			const auto& publicKey() const {
				return m_keyPair.publicKey();
			}

		private:
			crypto::KeyPair m_keyPair;
			test::LocalNodeTestState m_state;
			std::unique_ptr<plugins::PluginManager> m_pPluginManager;
			std::unique_ptr<cache::MemoryUtCache> m_pUnconfirmedTransactionsCache;
			NetworkChainHeight m_networkHeight;
			std::atomic<size_t> m_numConsumedBlockElements;
			std::atomic<size_t> m_numConsumedTransactionElements;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(NetworkPacketReaderServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootReaders<TestContext>([](const auto& context) {
			EXPECT_EQ(0u, context.numConsumedBlockElements());
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	TEST(NetworkPacketReaderServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownReaders<TestContext>([](const auto& context) {
			EXPECT_EQ(0u, context.numConsumedBlockElements());
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	// endregion

	// region basic connection

	TEST(NetworkPacketReaderServiceTests, CanAcceptExternalConnection) {
		// Act:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto&, const auto&) {
			// Assert: nothing was consumed
			EXPECT_EQ(0u, context.numConsumedBlockElements());
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	// endregion

	// region push entity handlers

	TEST(NetworkPacketReaderServiceTests, CanPushBlockWhenSynced) {
		// Arrange:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto& service, auto& io) {
			// Act: push a block
			io.write(test::GenerateRandomBlockPacket(), [](auto) {});
			WAIT_FOR_ONE_EXPR(context.numConsumedBlockElements());

			// Assert: a block was consumed
			EXPECT_EQ(1u, context.numConsumedBlockElements());
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
			EXPECT_EQ(1u, service.numActiveReaders());
		});
	}

	TEST(NetworkPacketReaderServiceTests, CanPushTransactionWhenSynced) {
		// Arrange:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto& service, auto& io) {
			// Act: push a transaction
			io.write(test::GenerateRandomTransactionPacket(), [](auto) {});
			WAIT_FOR_ONE_EXPR(context.numConsumedTransactionElements());

			// Assert: a transaction was consumed
			EXPECT_EQ(0u, context.numConsumedBlockElements());
			EXPECT_EQ(1u, context.numConsumedTransactionElements());
			EXPECT_EQ(1u, service.numActiveReaders());
		});
	}

	namespace {
		void AssertCannotPushWhenNotSynced(const std::shared_ptr<ionet::Packet>& pPacket) {
			// Arrange: set the chain height to 10 (local height is 1)
			test::RunSingleReaderTest(TestContext(Height(10)), [pPacket](const auto& context, const auto& service, auto& io) {
				// Act: push an entity
				io.write(pPacket, [](auto) {});

				// - since no consumer should be called, there is nothing to wait for, therefore need to rely on pause()
				//   being long enough to catch a bug where consumer is called
				test::Pause();

				// Assert: nothing was consumed
				EXPECT_EQ(0u, context.numConsumedBlockElements());
				EXPECT_EQ(0u, context.numConsumedTransactionElements());
				EXPECT_EQ(1u, service.numActiveReaders());
			});
		}
	}

	TEST(NetworkPacketReaderServiceTests, CannotPushBlockWhenNotSynced) {
		// Assert:
		AssertCannotPushWhenNotSynced(test::GenerateRandomBlockPacket());
	}

	TEST(NetworkPacketReaderServiceTests, CannotPushTransactionWhenNotSynced) {
		// Assert:
		AssertCannotPushWhenNotSynced(test::GenerateRandomTransactionPacket());
	}

	// endregion

	// region diagnostic handlers

	namespace {
		std::shared_ptr<ionet::Packet> GenerateSquaresPacket() {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(3 * sizeof(uint64_t));
			pPacket->Type = SquaresTraits::Packet_Type;

			auto* pData = reinterpret_cast<uint64_t*>(pPacket->Data());
			pData[0] = 3;
			pData[1] = 8;
			pData[2] = 5;
			return pPacket;
		}
	}

	TEST(NetworkPacketReaderServiceTests, CanPerformDiagnosticRequest) {
		// Arrange: make a simple squares request
		ionet::ByteBuffer packetBuffer;
		test::RunSingleReaderTest(TestContext(), [&packetBuffer](const auto&, const auto&, auto& io) {
			auto pRequestPacket = GenerateSquaresPacket();

			// Act: send a confirm timestamped hashes request
			io.write(pRequestPacket, [&io, &packetBuffer](auto) {
				test::AsyncReadIntoBuffer(io, packetBuffer);
			});
		});

		// Assert: the requested squared values should be returned
		auto pResponsePacket = reinterpret_cast<const ionet::PacketHeader*>(packetBuffer.data());
		ASSERT_TRUE(!!pResponsePacket);
		ASSERT_EQ(sizeof(ionet::PacketHeader) + 3 * sizeof(uint64_t), pResponsePacket->Size);
		EXPECT_EQ(static_cast<ionet::PacketType>(SquaresTraits::Packet_Type), pResponsePacket->Type);

		const auto* pData = reinterpret_cast<const uint64_t*>(pResponsePacket + 1);
		EXPECT_EQ(9u, pData[0]);
		EXPECT_EQ(64u, pData[1]);
		EXPECT_EQ(25u, pData[2]);
	}

	// endregion
}}}
