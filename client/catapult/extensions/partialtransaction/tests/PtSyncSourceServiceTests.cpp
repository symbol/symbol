#include "partialtransaction/src/PtSyncSourceService.h"
#include "partialtransaction/src/PtBootstrapperService.h"
#include "catapult/cache/MemoryPtCache.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace partialtransaction {

#define TEST_CLASS PtSyncSourceServiceTests

	namespace {
		struct PtSyncSourceServiceTraits {
			static constexpr auto CreateRegistrar = CreatePtSyncSourceServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<PtSyncSourceServiceTraits> {
		public:
			TestContext() {
				// Arrange: register service dependencies
				auto pBootstrapperRegistrar = CreatePtBootstrapperServiceRegistrar([]() {
					return std::make_unique<cache::MemoryPtCacheProxy>(cache::MemoryCacheOptions(100, 100));
				});
				pBootstrapperRegistrar->registerServices(locator(), testState().state());

				// - register hook dependencies
				auto& hooks = GetPtServerHooks(locator());
				hooks.setPtRangeConsumer([](auto&&) {});
				hooks.setCosignatureRangeConsumer([](auto&&) {});
			}
		};
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(PtSyncSource, Post_Extended_Range_Consumers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert: only dependency services are registered
		EXPECT_EQ(2u, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(3u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Push_Partial_Transactions));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Push_Detached_Cosignatures));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Pull_Partial_Transaction_Infos));
	}

	// endregion
}}
