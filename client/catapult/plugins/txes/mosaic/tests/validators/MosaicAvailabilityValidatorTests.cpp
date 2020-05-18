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
#include "src/cache/MosaicCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/constants.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicAvailability,)

	namespace {
		model::MosaicDefinitionNotification CreateNotification(const Key& owner, MosaicId id, const model::MosaicProperties& properties) {
			return model::MosaicDefinitionNotification(owner, id, properties);
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				Height height,
				const model::MosaicDefinitionNotification& notification) {
			// Arrange:
			auto pValidator = CreateMosaicAvailabilityValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, height);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", id " << notification.MosaicId;
		}

		void AddMosaic(cache::CatapultCache& cache, MosaicId id, Amount mosaicSupply, const Key& owner, Amount ownerSupply) {
			auto delta = cache.createDelta();
			test::AddMosaic(delta, id, Height(50), BlockDuration(100), mosaicSupply, owner);
			test::AddMosaicOwner(delta, id, owner, ownerSupply);
			cache.commit(Height());
		}

		void AddEternalMosaic(cache::CatapultCache& cache, MosaicId id, const Key& owner) {
			auto delta = cache.createDelta();
			test::AddEternalMosaic(delta, id, Height(50), owner);
			test::AddMosaicOwner(delta, id, owner, Amount());
			cache.commit(Height());
		}
	}

	// region unknown / inactive mosaic

	TEST(TEST_CLASS, SuccessWhenMosaicIsUnknown) {
		// Arrange:
		auto owner = test::GenerateRandomByteArray<Key>();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, 0);
		auto notification = CreateNotification(owner, MosaicId(123), properties);

		// - seed the cache with an unrelated mosaic
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(100), Amount(500), owner, Amount(400));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
	}

	TEST(TEST_CLASS, FailureWhenMosaicExistsButIsNotActive) {
		// Arrange:
		auto owner = test::GenerateRandomByteArray<Key>();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 3, 200);
		auto notification = CreateNotification(owner, MosaicId(123), properties);

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(0), owner, Amount(0));

		// Assert: mosaic expires at height 150
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(150), notification);
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(151), notification);
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(999), notification);
	}

	TEST(TEST_CLASS, FailureWhenNotificationOwnerIsNotMosaicOwner) {
		// Arrange:
		auto owner = test::GenerateRandomByteArray<Key>();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 3, 200);
		auto notification = CreateNotification(owner, MosaicId(123), properties);

		// - seed the cache with an active mosaic with the same id and zero supply
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(0), test::GenerateRandomByteArray<Key>(), Amount(0));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Owner_Conflict, cache, Height(100), notification);
	}

	// endregion

	// region properties check

	namespace {
		void AssertEternalPropertiesCheck(ValidationResult expectedResult, const model::MosaicProperties& properties) {
			// Arrange:
			auto owner = test::GenerateRandomByteArray<Key>();
			auto notification = CreateNotification(owner, MosaicId(123), properties);

			// - seed the cache with an active mosaic with the same id
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddEternalMosaic(cache, MosaicId(123), owner);

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenDefinitionIsChanged_Eternal) {
		AssertEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(1, 0, 0));
		AssertEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(0, 1, 0));
		AssertEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(1, 1, 0));
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenDefinitionIsUnchanged_Eternal) {
		// Assert: duration delta is not a pertinent change
		AssertEternalPropertiesCheck(Failure_Mosaic_Modification_No_Changes, test::CreateMosaicPropertiesFromValues(0, 0, 1));
		AssertEternalPropertiesCheck(Failure_Mosaic_Modification_No_Changes, test::CreateMosaicPropertiesFromValues(0, 0, 0));
	}

	namespace {
		void AssertNonEternalPropertiesCheck(ValidationResult expectedResult, const model::MosaicProperties& properties) {
			// Arrange:
			auto owner = test::GenerateRandomByteArray<Key>();
			auto notification = CreateNotification(owner, MosaicId(123), properties);

			// - seed the cache with an active mosaic with the same id and a lifetime of 100 blocks
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(0), owner, Amount(0));

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenDefinitionIsChanged_NonEternal) {
		AssertNonEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(1, 0, 0));
		AssertNonEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(0, 1, 0));
		AssertNonEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(0, 0, 1));
		AssertNonEternalPropertiesCheck(ValidationResult::Success, test::CreateMosaicPropertiesFromValues(1, 1, 1));
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenDefinitionIsUnchanged_NonEternal) {
		AssertNonEternalPropertiesCheck(Failure_Mosaic_Modification_No_Changes, test::CreateMosaicPropertiesFromValues(0, 0, 0));
	}

	// endregion

	// region supply check

	namespace {
		void AssertCanReplaceActiveMosaicWhenSupplyIsZero(uint8_t divisibility) {
			// Arrange:
			auto owner = test::GenerateRandomByteArray<Key>();
			auto properties = test::CreateMosaicPropertiesFromValues(0, divisibility, 200);
			auto notification = CreateNotification(owner, MosaicId(123), properties);

			// - seed the cache with an active mosaic with the same id and zero supply
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(0), owner, Amount(0));

			// Assert:
			AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenSupplyIsZero_RequiredPropertiesChanged) {
		AssertCanReplaceActiveMosaicWhenSupplyIsZero(3);
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenSupplyIsZero_RequiredPropertiesUnchanged) {
		AssertCanReplaceActiveMosaicWhenSupplyIsZero(0);
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenSupplyIsNonzero) {
		// Arrange:
		auto owner = test::GenerateRandomByteArray<Key>();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, 200);
		auto notification = CreateNotification(owner, MosaicId(123), properties);

		// - seed the cache with an active mosaic with the same id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(100), owner, Amount(100));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_Disallowed, cache, Height(100), notification);
	}

	// endregion
}}
