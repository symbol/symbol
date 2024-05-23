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

#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockchainConfiguration.h"
#include "catapult/validators/ValidatorContext.h"
#include "src/validators/Validators.h"
#include "tests/TestHarness.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

namespace catapult {
namespace validators {

#define TEST_CLASS EligibleHarvesterValidatorTests

    DEFINE_COMMON_VALIDATOR_TESTS(EligibleHarvester, )

    namespace {
        constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
        constexpr auto Importance_Grouping = 234u;

        constexpr auto Success_Result = ValidationResult::Success;
        constexpr auto Failure_Result = Failure_Core_Block_Harvester_Ineligible;

        // region test utils

        auto ConvertToImportanceHeight(Height height)
        {
            return model::ConvertToImportanceHeight(height, Importance_Grouping);
        }

        auto CreateEmptyCatapultCache(Amount minHarvesterBalance, Amount maxHarvesterBalance)
        {
            auto config = model::BlockchainConfiguration::Uninitialized();
            config.HarvestingMosaicId = Harvesting_Mosaic_Id;
            config.ImportanceGrouping = Importance_Grouping;
            config.MinHarvesterBalance = minHarvesterBalance;
            config.MaxHarvesterBalance = maxHarvesterBalance;
            return test::CreateEmptyCatapultCache(config);
        }

        void AddAccount(
            cache::CatapultCache& cache,
            const Address& harvester,
            Importance importance,
            model::ImportanceHeight importanceHeight,
            Amount balance)
        {
            auto delta = cache.createDelta();
            auto& accountStateCache = delta.sub<cache::AccountStateCache>();
            accountStateCache.addAccount(harvester, Height(100));
            auto& accountState = accountStateCache.find(harvester).get();
            accountState.ImportanceSnapshots.set(importance, importanceHeight);
            accountState.Balances.credit(Harvesting_Mosaic_Id, balance);
            cache.commit(Height());
        }

        void AssertValidationResult(
            ValidationResult expectedResult,
            int64_t minBalanceDelta,
            Importance importance,
            model::ImportanceHeight importanceHeight,
            Height blockHeight)
        {
            // Arrange:
            auto cache = CreateEmptyCatapultCache(Amount(1234), Amount(9876));
            auto harvesterAddress = test::GenerateRandomByteArray<Address>();
            auto initialBalance = Amount(static_cast<Amount::ValueType>(1234 + minBalanceDelta));
            AddAccount(cache, harvesterAddress, importance, importanceHeight, initialBalance);

            auto pValidator = CreateEligibleHarvesterValidator();
            auto notification = test::CreateBlockNotification(harvesterAddress);

            // Act:
            auto result = test::ValidateNotification(*pValidator, notification, cache, blockHeight);

            // Assert:
            EXPECT_EQ(expectedResult, result);
        }

        void AssertValidationResult(ValidationResult expectedResult, int64_t minBalanceDelta, Importance importance)
        {
            auto height = Height(10000);
            AssertValidationResult(expectedResult, minBalanceDelta, importance, ConvertToImportanceHeight(height), height);
        }

        // endregion
    }

    // region tests

    TEST(TEST_CLASS, FailureWhenAccountIsUnknown)
    {
        // Arrange:
        auto cache = CreateEmptyCatapultCache(Amount(0), Amount(100'000));
        auto otherAccountAddress = test::GenerateRandomByteArray<Address>();
        auto height = Height(1000);
        AddAccount(cache, otherAccountAddress, Importance(1000), ConvertToImportanceHeight(height), Amount(9999));

        auto pValidator = CreateEligibleHarvesterValidator();

        auto harvesterAddress = test::GenerateRandomByteArray<Address>();
        auto notification = test::CreateBlockNotification(harvesterAddress);

        // Act:
        auto result = test::ValidateNotification(*pValidator, notification, cache, height);

        // Assert:
        EXPECT_EQ(Failure_Result, result);
    }

    TEST(TEST_CLASS, FailureWhenBalanceIsBelowMinBalance)
    {
        AssertValidationResult(Failure_Result, -1, Importance(200));
        AssertValidationResult(Failure_Result, -100, Importance(200));
    }

    TEST(TEST_CLASS, FailureWhenBalanceIsAboveMaxBalance)
    {
        AssertValidationResult(Failure_Result, 9876 - 1234 + 1, Importance(200));
        AssertValidationResult(Failure_Result, 12345, Importance(200));
    }

    TEST(TEST_CLASS, FailureWhenImportanceIsZero)
    {
        AssertValidationResult(Failure_Result, 2345, Importance(0));
    }

    TEST(TEST_CLASS, FailureWhenImportanceIsNotSetAtCorrectHeight)
    {
        AssertValidationResult(Failure_Result, 2345, Importance(200), model::ImportanceHeight(123), Height(1234));
    }

    TEST(TEST_CLASS, SuccessWhenAllCriteriaAreMet)
    {
        AssertValidationResult(Success_Result, 0, Importance(200));
        AssertValidationResult(Success_Result, 1, Importance(200));
        AssertValidationResult(Success_Result, 2345, Importance(200));
        AssertValidationResult(Success_Result, 9876 - 1234, Importance(200));
    }

    TEST(TEST_CLASS, SuccessWhenValidatingNemesisBlock)
    {
        AssertValidationResult(Success_Result, 2345, Importance(0), model::ImportanceHeight(123), Height(1));
    }

    // endregion
}
}
