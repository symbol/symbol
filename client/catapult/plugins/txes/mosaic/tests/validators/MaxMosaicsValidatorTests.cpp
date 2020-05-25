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
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(MaxMosaicsBalanceTransfer, 123)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxMosaicsSupplyChange, 123)

#define BALANCE_TRANSFER_TEST_CLASS BalanceTransferMaxMosaicsValidatorTests
#define SUPPLY_CHANGE_TEST_CLASS SupplyChangeMaxMosaicsValidatorTests

	namespace {
		template<typename TAccountIdentifier>
		auto CreateAndSeedCache(const TAccountIdentifier& accountIdentifier) {
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(accountIdentifier, Height());
				auto& accountState = accountStateCacheDelta.find(accountIdentifier).get();
				for (auto i = 0u; i < 5; ++i)
					accountState.Balances.credit(MosaicId(i + 1), Amount(1));

				cache.commit(Height());
			}

			return cache;
		}

		void RunBalanceTransferTest(ValidationResult expectedResult, uint16_t maxMosaics, UnresolvedMosaicId mosaicId, Amount amount) {
			// Arrange:
			auto recipient = test::GenerateRandomByteArray<Address>();
			auto unresolvedRecipient = test::UnresolveXor(recipient);
			auto cache = CreateAndSeedCache(recipient);

			auto pValidator = CreateMaxMosaicsBalanceTransferValidator(maxMosaics);
			auto notification = model::BalanceTransferNotification(Address(), unresolvedRecipient, mosaicId, amount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "maxMosaics " << maxMosaics << ", mosaicId " << mosaicId << ", amount " << amount;
		}
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, FailureWhenMaximumIsExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 1, test::UnresolveXor(MosaicId(6)), Amount(100));
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 4, test::UnresolveXor(MosaicId(6)), Amount(100));
		RunBalanceTransferTest(Failure_Mosaic_Max_Mosaics_Exceeded, 5, test::UnresolveXor(MosaicId(6)), Amount(100));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessWhenAmountIsZero) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 5, test::UnresolveXor(MosaicId(6)), Amount(0));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessWhenAccountAlreadyOwnsThatMosaic) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 5, test::UnresolveXor(MosaicId(3)), Amount(100));
	}

	TEST(BALANCE_TRANSFER_TEST_CLASS, SuccessWhenMaximumIsNotExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		RunBalanceTransferTest(ValidationResult::Success, 6, test::UnresolveXor(MosaicId(6)), Amount(100));
		RunBalanceTransferTest(ValidationResult::Success, 10, test::UnresolveXor(MosaicId(6)), Amount(100));
		RunBalanceTransferTest(ValidationResult::Success, 123, test::UnresolveXor(MosaicId(6)), Amount(100));
	}

	namespace {
		void RunMosaicSupplyTest(
				ValidationResult expectedResult,
				uint16_t maxMosaics,
				UnresolvedMosaicId mosaicId,
				model::MosaicSupplyChangeAction action) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto cache = CreateAndSeedCache(owner);

			auto pValidator = CreateMaxMosaicsSupplyChangeValidator(maxMosaics);
			auto notification = model::MosaicSupplyChangeNotification(owner, mosaicId, action, Amount(100));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "maxMosaics " << maxMosaics
					<< ", mosaicId " << mosaicId
					<< ", action " << utils::to_underlying_type(action);
		}
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, FailureWhenMaximumIsExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		auto action = model::MosaicSupplyChangeAction::Increase;
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 1, test::UnresolveXor(MosaicId(6)), action);
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 4, test::UnresolveXor(MosaicId(6)), action);
		RunMosaicSupplyTest(Failure_Mosaic_Max_Mosaics_Exceeded, 5, test::UnresolveXor(MosaicId(6)), action);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessWhenSupplyIsDecreased) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		auto action = model::MosaicSupplyChangeAction::Decrease;
		RunMosaicSupplyTest(ValidationResult::Success, 5, test::UnresolveXor(MosaicId(6)), action);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessWhenAccountAlreadyOwnsThatMosaic) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		auto action = model::MosaicSupplyChangeAction::Increase;
		RunMosaicSupplyTest(ValidationResult::Success, 5, test::UnresolveXor(MosaicId(3)), action);
	}

	TEST(SUPPLY_CHANGE_TEST_CLASS, SuccessWhenMaximumIsNotExceeded) {
		// Act: account in test already owns 5 mosaics with ids 1 to 5
		auto action = model::MosaicSupplyChangeAction::Increase;
		RunMosaicSupplyTest(ValidationResult::Success, 6, test::UnresolveXor(MosaicId(6)), action);
		RunMosaicSupplyTest(ValidationResult::Success, 10, test::UnresolveXor(MosaicId(6)), action);
		RunMosaicSupplyTest(ValidationResult::Success, 123, test::UnresolveXor(MosaicId(6)), action);
	}
}}
