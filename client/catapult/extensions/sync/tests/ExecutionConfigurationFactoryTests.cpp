#include "sync/src/ExecutionConfigurationFactory.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS ExecutionConfigurationFactoryTests

	TEST(TEST_CLASS, CanCreateExecutionConfiguration) {
		// Act:
		auto config = CreateExecutionConfiguration(*test::CreateDefaultPluginManager());

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, config.Network.Identifier);
		EXPECT_TRUE(!!config.pObserver);
		EXPECT_TRUE(!!config.pValidator);
		EXPECT_TRUE(!!config.pNotificationPublisher);

		// - notice that only observers and validators registered in CreateDefaultPluginManager are present
		std::vector<std::string> expectedObserverNames{
			"AccountAddressObserver",
			"AccountPublicKeyObserver",
			"BalanceObserver",
			"HarvestFeeObserver",
			"RecalculateImportancesObserver",
			"BlockDifficultyObserver",
			"BlockDifficultyPruningObserver"
		};
		EXPECT_EQ(expectedObserverNames, config.pObserver->names());

		std::vector<std::string> expectedValidatorNames{
			"DeadlineValidator",
			"NemesisSinkValidator",
			"EligibleHarvesterValidator",
			"BalanceReserveValidator",
			"BalanceTransferValidator"
		};
		EXPECT_EQ(expectedValidatorNames, config.pValidator->names());
	}
}}
