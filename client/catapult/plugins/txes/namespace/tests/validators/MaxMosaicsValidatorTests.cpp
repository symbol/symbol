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
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(MaxMosaicsBalanceTransfer, 123)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxMosaicsSupplyChange, 123)

#define BALANCE_TRANSFER_TEST_CLASS BalanceTransferMaxMosaicsValidatorTests
#define SUPPLY_CHANGE_TEST_CLASS SupplyChangeMaxMosaicsValidatorTests

	namespace {
		template<typename TKey>
		auto CreateAndSeedCache(const TKey& key) {
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(key, Height());
				auto& accountState = accountStateCacheDelta.find(key).get();
				for (auto i = 0u; i < 5; ++i)
					accountState.Balances.credit(MosaicId(i + 1), Amount(1));

				cache.commit(Height());
			}

			return cache;
		}

		void RunBalanceTransferTest(ValidationResult expectedResult, uint16_t maxMosaics, MosaicId mosaicId, Amount amount) {
			// Arrange:
			auto owner = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
			auto cache = CreateAndSeedCache(recipient);

			auto pValidator = CreateMaxMosaicsBalanceTransferValidator(maxMosaics);
			auto notification = model::BalanceTransferNotification(owner, recipient, mosaicId, amount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "maxMosaics " << maxMosaics << ", mosaicId " << mosaicId << ", amount " << amount;
		}
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, FailureIfMaximumIsExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 1, MosaicId(6), Amount(100));
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 4, MosaicId(6), Amount(100));
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 5, MosaicId(6), Amount(100));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessIfAmountIsZero) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 5, MosaicId(6), Amount(0));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessIfAccountAlreadyOwnsThatMosaic) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 5, MosaicId(3), Amount(100));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessIfMaximumIsNotExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 6, MosaicId(6), Amount(100));
		RunBalanceTransferTest(ValidationResult::Success, 10, MosaicId(6), Amount(100));
		RunBalanceTransferTest(ValidationResult::Success, 123, MosaicId(6), Amount(100));
	}

	namespace {
		void RunMosaicSupplyTest(
				ValidationResult expectedResult,
				uint16_t maxMosaics,
				MosaicId mosaicId,
				model::MosaicSupplyChangeDirection direction) {
			// Arrange:
			auto owner = test::GenerateRandomData<Key_Size>();
			auto cache = CreateAndSeedCache(owner);

			auto pValidator = CreateMaxMosaicsSupplyChangeValidator(maxMosaics);
			auto notification = model::MosaicSupplyChangeNotification(owner, mosaicId, direction, Amount(100));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "maxMosaics " << maxMosaics
					<< ", mosaicId " << mosaicId
					<< ", direction " << utils::to_underlying_type(direction);
		}
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, FailureIfMaximumIsExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 1, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 4, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 5, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessIfSupplyIsDecreased) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunMosaicSupplyTest(ValidationResult::Success, 5, MosaicId(6), model::MosaicSupplyChangeDirection::Decrease);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessIfAccountAlreadyOwnsThatMosaic) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunMosaicSupplyTest(ValidationResult::Success, 5, MosaicId(3), model::MosaicSupplyChangeDirection::Increase);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessIfMaximumIsNotExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunMosaicSupplyTest(ValidationResult::Success, 6, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
		RunMosaicSupplyTest(ValidationResult::Success, 10, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
		RunMosaicSupplyTest(ValidationResult::Success, 123, MosaicId(6), model::MosaicSupplyChangeDirection::Increase);
	}
}}
