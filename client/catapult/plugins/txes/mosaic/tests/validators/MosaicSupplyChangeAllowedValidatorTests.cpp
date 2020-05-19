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
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicSupplyChangeAllowedValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicSupplyChangeAllowed, Amount())

	namespace {
		constexpr auto Max_Atomic_Units = Amount(std::numeric_limits<Amount::ValueType>::max());

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				Height height,
				const model::MosaicSupplyChangeNotification& notification,
				Amount maxAtomicUnits = Max_Atomic_Units) {
			// Arrange:
			auto pValidator = CreateMosaicSupplyChangeAllowedValidator(maxAtomicUnits);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, height);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "id " << notification.MosaicId << ", delta " << notification.Delta;
		}

		void AddMosaic(
				cache::CatapultCache& cache,
				MosaicId id,
				Amount mosaicSupply,
				const Address& owner,
				Amount ownerSupply,
				model::MosaicFlags flags = model::MosaicFlags::Supply_Mutable) {
			auto delta = cache.createDelta();

			// add a mosaic definition with the desired flags
			model::MosaicProperties properties(flags, 0, BlockDuration());

			auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
			auto definition = state::MosaicDefinition(Height(50), owner, 3, properties);
			auto entry = state::MosaicEntry(id, definition);
			entry.increaseSupply(mosaicSupply);
			mosaicCacheDelta.insert(entry);

			test::AddMosaicOwner(delta, id, owner, ownerSupply);
			cache.commit(Height());
		}
	}

	// region immutable supply

	namespace {
		void AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeAction action) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto notification = model::MosaicSupplyChangeNotification(owner, test::UnresolveXor(MosaicId(123)), action, Amount(100));

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(500), owner, Amount(500), model::MosaicFlags::None);

			// Assert:
			AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanIncreaseImmutableSupplyWhenOwnerHasCompleteSupply) {
		AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeAction::Increase);
	}

	TEST(TEST_CLASS, CanDecreaseImmutableSupplyWhenOwnerHasCompleteSupply) {
		AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeAction::Decrease);
	}

	namespace {
		void AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeAction action) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto notification = model::MosaicSupplyChangeNotification(owner, test::UnresolveXor(MosaicId(123)), action, Amount(100));

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(500), owner, Amount(499), model::MosaicFlags::None);

			// Assert:
			AssertValidationResult(Failure_Mosaic_Supply_Immutable, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CannotIncreaseImmutableSupplyWhenOwnerHasPartialSupply) {
		AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeAction::Increase);
	}

	TEST(TEST_CLASS, CannotDecreaseImmutableSupplyWhenOwnerHasPartialSupply) {
		AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeAction::Decrease);
	}

	// endregion

	// region decrease supply

	namespace {
		void AssertDecreaseValidationResult(ValidationResult expectedResult, Amount mosaicSupply, Amount ownerSupply, Amount delta) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto action = model::MosaicSupplyChangeAction::Decrease;
			auto notification = model::MosaicSupplyChangeNotification(owner, test::UnresolveXor(MosaicId(123)), action, delta);

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), mosaicSupply, owner, ownerSupply);

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanDecreaseMutableSupplyByLessThanOwnerSupply) {
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(300));
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(399));
	}

	TEST(TEST_CLASS, CanDecreaseMutableSupplyByEntireOwnerSupply) {
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(400));
	}

	TEST(TEST_CLASS, CannotDecreaseMutableSupplyByGreaterThanOwnerSupply) {
		AssertDecreaseValidationResult(Failure_Mosaic_Supply_Negative, Amount(500), Amount(400), Amount(401));
		AssertDecreaseValidationResult(Failure_Mosaic_Supply_Negative, Amount(500), Amount(400), Amount(500));
	}

	// endregion

	// region increase

	namespace {
		void AssertIncreaseValidationResult(ValidationResult expectedResult, Amount maxAtomicUnits, Amount mosaicSupply, Amount delta) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto action = model::MosaicSupplyChangeAction::Increase;
			auto notification = model::MosaicSupplyChangeNotification(owner, test::UnresolveXor(MosaicId(123)), action, delta);

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), mosaicSupply, owner, Amount(111));

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification, maxAtomicUnits);
		}
	}

	TEST(TEST_CLASS, CanIncreaseMutableSupplyToLessThanAtomicUnits) {
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(300));
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(399));
	}

	TEST(TEST_CLASS, CanIncreaseMutableSupplyToExactlyAtomicUnits) {
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(400));
	}

	TEST(TEST_CLASS, CannotIncreaseMutableSupplyToGreaterThanAtomicUnits) {
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Amount(401));
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, CannotIncreaseMutableSupplyWhenOverflowIsDetected) {
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Max_Atomic_Units);
	}

	// endregion
}}
