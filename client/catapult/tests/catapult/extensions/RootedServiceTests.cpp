#include "catapult/extensions/RootedService.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS RootedServiceTests

	namespace {
		struct RootedServiceTraits {
			static auto CreateRegistrar(const std::shared_ptr<void>& pService, const std::string& serviceName) {
				return CreateRootedServiceRegistrar(pService, serviceName, ServiceRegistrarPhase::Initial);
			}
		};

		using TestContext = test::ServiceLocatorTestContext<RootedServiceTraits>;
	}

	TEST(TEST_CLASS, ServiceRegistrarSelfReportsCorrectInformation) {
		// Arrange:
		auto pService = CreateRootedServiceRegistrar(std::make_shared<int>(3), "ALPHA", ServiceRegistrarPhase::Initial_With_Modules);

		// Act:
		auto info = pService->info();

		// Assert:
		EXPECT_EQ("Rooted - ALPHA", info.Name);
		EXPECT_EQ(ServiceRegistrarPhase::Initial_With_Modules, info.Phase);
	}

	TEST(TEST_CLASS, NoCountersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(std::make_shared<int>(3), "ALPHA");

		// Assert:
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	TEST(TEST_CLASS, SingleServiceIsRegistered) {
		// Arrange:
		TestContext context;
		auto pService = std::make_shared<int>(3);

		// Act:
		context.boot(pService, "ALPHA");

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(pService, context.locator().service<int>("ALPHA"));
	}
}}
