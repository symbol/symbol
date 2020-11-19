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

#include "src/validators/Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ImportanceBlockValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ImportanceBlock,)

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Failure_Result = Failure_Core_Importance_Block_Mismatch;

		// region test utils

		template<typename TNotificationModifier>
		void AssertValidationResult(ValidationResult expectedResult, TNotificationModifier modifier) {
			// Arrange:
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = 234;
			config.MinHarvesterBalance = Amount(1'000'000);
			config.MaxHarvesterBalance = Amount(100'000'000);
			config.MinVoterBalance = Amount(1'100'000);
			auto cache = test::CreateEmptyCatapultCache(config);

			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				test::AddAccountsWithBalances(accountStateCacheDelta, Harvesting_Mosaic_Id, {
					Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000)
				});
				accountStateCacheDelta.updateHighValueAccounts(Height(1));
				cache.commit(Height(1));
			}

			auto pValidator = CreateImportanceBlockValidator();

			auto previousHash = test::GenerateRandomByteArray<Hash256>();
			auto notification = model::ImportanceBlockNotification(2, 3, Amount(2'300'000), previousHash);
			modifier(notification);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		// endregion
	}

	// region tests

	TEST(TEST_CLASS, SuccessWhenAllFieldsMatch) {
		AssertValidationResult(ValidationResult::Success, [](const auto&) {});
	}

	TEST(TEST_CLASS, FailureWhenFieldMismatch_VotingEligibleAccountsCount) {
		AssertValidationResult(Failure_Result, [](auto& notification) { --notification.VotingEligibleAccountsCount; });
		AssertValidationResult(Failure_Result, [](auto& notification) { ++notification.VotingEligibleAccountsCount; });
	}

	TEST(TEST_CLASS, FailureWhenFieldMismatch_HarvestingEligibleAccountsCount) {
		AssertValidationResult(Failure_Result, [](auto& notification) { --notification.HarvestingEligibleAccountsCount; });
		AssertValidationResult(Failure_Result, [](auto& notification) { ++notification.HarvestingEligibleAccountsCount; });
	}

	TEST(TEST_CLASS, FailureWhenFieldMismatch_TotalVotingBalance) {
		AssertValidationResult(Failure_Result, [](auto& notification) {
			notification.TotalVotingBalance = notification.TotalVotingBalance - Amount(1);
		});
		AssertValidationResult(Failure_Result, [](auto& notification) {
			notification.TotalVotingBalance = notification.TotalVotingBalance + Amount(1);
		});
	}

	// endregion
}}
