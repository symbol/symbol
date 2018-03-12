#include "partialtransaction/src/PtService.h"
#include "partialtransaction/src/PtBootstrapperService.h"
#include "catapult/cache/MemoryPtCache.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/local/PacketWritersServiceTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace partialtransaction {

#define TEST_CLASS PtServiceTests

	namespace {
		constexpr auto Num_Expected_Tasks = 2u;

		struct PtServiceTraits {
			static constexpr auto Counter_Name = "PT WRITERS";
			static constexpr auto Num_Expected_Services = 3; // writers (1) + dependent services (2)

			static auto GetWriters(const extensions::ServiceLocator& locator) {
				return locator.service<net::PacketWriters>("api.partial");
			}

			static auto CreateRegistrar() {
				return CreatePtServiceRegistrar();
			}
		};

		class TestContext : public test::ServiceLocatorTestContext<PtServiceTraits> {
		public:
			TestContext() {
				// Arrange: register service dependencies
				auto pBootstrapperRegistrar = CreatePtBootstrapperServiceRegistrar([]() {
					return std::make_unique<cache::MemoryPtCacheProxy>(cache::MemoryCacheOptions(100, 100));
				});
				pBootstrapperRegistrar->registerServices(locator(), testState().state());

				// - register hook dependencies
				GetPtServerHooks(locator()).setCosignedTransactionInfosConsumer([](auto&&) {});
			}
		};

		struct Mixin {
			using TraitsType = PtServiceTraits;
			using TestContextType = TestContext;
		};
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Pt, Post_Extended_Range_Consumers)

	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanBootService)
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanShutdownService)
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanConnectToExternalServer)

	// region packetIoPickers

	TEST(TEST_CLASS, WritersAreRegisteredInPacketIoPickers) {
		// Arrange: create a (tcp) server
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto serverKeyPair = test::GenerateKeyPair();
		test::SpawnPacketServerWork(pPool->service(), [&service = pPool->service(), &serverKeyPair](const auto& pServer) {
			net::VerifyClient(pServer, serverKeyPair, [&service](auto, const auto&) {});
		});

		// Act: create and boot the service
		TestContext context;
		context.boot();
		auto pickers = context.testState().state().packetIoPickers();

		// - get the packet writers and attempt to connect to the server
		test::ConnectToLocalHost(*PtServiceTraits::GetWriters(context.locator()), serverKeyPair.publicKey());

		// Assert: the writers are registered with role `Api`
		EXPECT_EQ(0u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Peer).size());
		EXPECT_EQ(1u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Api).size());
	}

	// endregion

	// region tasks

	TEST(TEST_CLASS, ConnectPeersTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), Num_Expected_Tasks, "connect peers task for service Pt");
	}

	TEST(TEST_CLASS, PullPtTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), Num_Expected_Tasks, "pull partial transactions task");
	}

	// endregion
}}
