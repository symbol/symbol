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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicAvailability,)

	namespace {
		model::MosaicDefinitionNotification CreateNotification(
				const Key& signer,
				MosaicId id,
				uint8_t divisibility,
				BlockDuration duration) {
			auto properties = model::MosaicProperties::FromValues({ { 0, divisibility, duration.unwrap() } });
			return model::MosaicDefinitionNotification(signer, id, properties);
		}

		model::MosaicDefinitionNotification CreateNotification(const Key& signer, MosaicId id, BlockDuration duration) {
			return CreateNotification(signer, id, 0, duration);
		}

		model::MosaicDefinitionNotification CreateNotification(const Key& signer, MosaicId id) {
			return CreateNotification(signer, id, 0, Eternal_Artifact_Duration);
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
			test::AddMosaic(delta, id, Height(50), BlockDuration(100), mosaicSupply);
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

	// region unknown mosaic

	TEST(TEST_CLASS, CanAddUnknownMosaic) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto notification = CreateNotification(signer, MosaicId(123));

		// - seed the cache with an unrelated mosaic
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(100), Amount(500), signer, Amount(400));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
	}

	// endregion

	// region active mosaic check

	TEST(TEST_CLASS, FailureWhenMosaicExistsButIsNotActive) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto notification = CreateNotification(signer, MosaicId(123));

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(0), signer, Amount(0));

		// Assert: mosaic expires at height 150
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(150), notification);
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(151), notification);
		AssertValidationResult(Failure_Mosaic_Expired, cache, Height(999), notification);
	}

	// endregion

	// region properties check

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenDefinitionIsUnchanged) {
		// Arrange: create a transaction with matching properties
		auto signer = test::GenerateRandomByteArray<Key>();
		auto notification = CreateNotification(signer, MosaicId(123), BlockDuration(0));

		// - seed the cache with an active mosaic with the same id (notice that added mosaic has duration of 100)
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, MosaicId(123), signer);

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_No_Changes, cache, Height(100), notification);
	}

	// endregion

	// region supply check

	namespace {
		void AssertCanReplaceActiveMosaicWhenSupplyIsZero(uint8_t divisibility) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			auto notification = CreateNotification(signer, MosaicId(123), divisibility, BlockDuration(200));

			// - seed the cache with an active mosaic with the same id and zero supply
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(0), signer, Amount(0));

			// Assert:
			AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenSupplyIsZero_RequiredPropertiesChanged) {
		// Assert:
		AssertCanReplaceActiveMosaicWhenSupplyIsZero(3);
	}

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenSupplyIsZero_RequiredPropertiesUnchanged) {
		// Assert:
		AssertCanReplaceActiveMosaicWhenSupplyIsZero(0);
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenSupplyIsNonZero) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto notification = CreateNotification(signer, MosaicId(123), BlockDuration(200));

		// - seed the cache with an active mosaic with the same id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(100), signer, Amount(100));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_Disallowed, cache, Height(100), notification);
	}

	// endregion
}}
