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

#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ExecutionConfigurationFactoryTests

	TEST(TEST_CLASS, CanCreateExecutionConfiguration) {
		// Act:
		auto pPluginManager = test::CreateDefaultPluginManagerWithRealPlugins();
		auto config = CreateExecutionConfiguration(*pPluginManager);

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Private_Test, config.Network.Identifier);
		EXPECT_TRUE(!!config.pObserver);
		EXPECT_TRUE(!!config.pValidator);
		EXPECT_TRUE(!!config.pNotificationPublisher);
		EXPECT_TRUE(!!config.ResolverContextFactory);

		// - notice that only observers and validators registered in CreateDefaultPluginManagerWithRealPlugins are present
		std::vector<std::string> expectedObserverNames{
			"SourceChangeObserver",
			"AccountAddressObserver",
			"AccountPublicKeyObserver",
			"BalanceDebitObserver",
			"BalanceTransferObserver",
			"BeneficiaryObserver",
			"TransactionFeeActivityObserver",
			"HarvestFeeObserver",
			"TotalTransactionsObserver",
			"HighValueAccountCommitObserver",
			"VrfKeyLinkObserver",
			"VotingMultiKeyLinkObserver",
			"RecalculateImportancesObserver",
			"HighValueAccountRollbackObserver",
			"BlockStatisticObserver"
		};
		EXPECT_EQ(expectedObserverNames, config.pObserver->names());

		std::vector<std::string> expectedValidatorNames{
			"AddressValidator",
			"PublicKeyValidator",
			"DeadlineValidator",
			"NemesisSinkValidator",
			"EligibleHarvesterValidator",
			"BalanceDebitValidator",
			"BalanceTransferValidator",
			"VrfKeyLinkValidator",
			"VotingMultiKeyLinkValidator"
		};
		EXPECT_EQ(expectedValidatorNames, config.pValidator->names());
	}
}}
