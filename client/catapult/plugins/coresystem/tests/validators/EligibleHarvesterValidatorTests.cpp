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

#include "src/validators/Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS EligibleHarvesterValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(EligibleHarvester, Amount(1234))

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Importance_Grouping = 234u;

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Importance_Grouping);
		}

		auto CreateEmptyCatapultCache() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = Importance_Grouping;
			return test::CreateEmptyCatapultCache(config);
		}

		void AddAccount(
				cache::CatapultCache& cache,
				const Key& publicKey,
				Importance importance,
				model::ImportanceHeight importanceHeight,
				Amount balance) {
			auto delta = cache.createDelta();
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(publicKey, Height(100));
			auto& accountState = accountStateCache.find(publicKey).get();
			accountState.ImportanceInfo.set(importance, importanceHeight);
			accountState.Balances.credit(Harvesting_Mosaic_Id, balance);
			cache.commit(Height());
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsUnknown) {
		// Arrange:
		auto cache = CreateEmptyCatapultCache();
		auto key = test::GenerateRandomByteArray<Key>();
		auto height = Height(1000);
		AddAccount(cache, key, Importance(1000), ConvertToImportanceHeight(height), Amount(9999));

		auto pValidator = CreateEligibleHarvesterValidator(Amount(1234));

		auto signer = test::GenerateRandomByteArray<Key>();
		auto notification = test::CreateBlockNotification(signer);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification, cache, height);

		// Assert:
		EXPECT_EQ(Failure_Core_Block_Harvester_Ineligible, result);
	}

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				int64_t minBalanceDelta,
				Importance importance,
				model::ImportanceHeight importanceHeight,
				Height blockHeight) {
			// Arrange:
			auto cache = CreateEmptyCatapultCache();
			auto key = test::GenerateRandomByteArray<Key>();
			auto initialBalance = Amount(static_cast<Amount::ValueType>(1234 + minBalanceDelta));
			AddAccount(cache, key, importance, importanceHeight, initialBalance);

			auto pValidator = CreateEligibleHarvesterValidator(Amount(1234));
			auto notification = test::CreateBlockNotification(key);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, blockHeight);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenBalanceIsBelowMinBalance) {
		// Assert:
		constexpr auto expectedResult = Failure_Core_Block_Harvester_Ineligible;
		auto height = Height(10000);
		AssertValidationResult(expectedResult, -1, Importance(123), ConvertToImportanceHeight(height), height);
		AssertValidationResult(expectedResult, -100, Importance(123), ConvertToImportanceHeight(height), height);
	}

	TEST(TEST_CLASS, FailureWhenImportanceIsZero) {
		// Assert:
		auto height = Height(10000);
		AssertValidationResult(Failure_Core_Block_Harvester_Ineligible, 12345, Importance(0), ConvertToImportanceHeight(height), height);
	}

	TEST(TEST_CLASS, FailureWhenImportanceIsNotSetAtCorrectHeight) {
		// Assert:
		AssertValidationResult(Failure_Core_Block_Harvester_Ineligible, 12345, Importance(0), model::ImportanceHeight(123), Height(1234));
	}

	TEST(TEST_CLASS, SuccessWhenAllCriteriaAreMet) {
		// Assert:
		constexpr auto expectedResult = ValidationResult::Success;
		auto height = Height(10000);
		AssertValidationResult(expectedResult, 0, Importance(123), ConvertToImportanceHeight(height), height);
		AssertValidationResult(expectedResult, 1, Importance(123), ConvertToImportanceHeight(height), height);
		AssertValidationResult(expectedResult, 12345, Importance(123), ConvertToImportanceHeight(height), height);
	}
}}
