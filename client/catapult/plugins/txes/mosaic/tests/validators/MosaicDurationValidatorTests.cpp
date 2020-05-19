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
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicDurationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicDuration, BlockDuration(123))

	namespace {
		constexpr MosaicId Default_Mosaic_Id = MosaicId(0x1234);

		model::MosaicDefinitionNotification CreateNotification(const Address& owner, const model::MosaicProperties& properties) {
			return model::MosaicDefinitionNotification(owner, Default_Mosaic_Id, properties);
		}

		void AddMosaic(cache::CatapultCache& cache, const Address& owner, BlockDuration duration) {
			auto delta = cache.createDelta();
			test::AddMosaic(delta, Default_Mosaic_Id, Height(50), duration, owner);
			cache.commit(Height());
		}

		void AddEternalMosaic(cache::CatapultCache& cache, const Address& owner) {
			auto delta = cache.createDelta();
			test::AddEternalMosaic(delta, Default_Mosaic_Id, Height(50), owner);
			cache.commit(Height());
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const model::MosaicDefinitionNotification& notification,
				Height height = Height(50)) {
			// Arrange:
			auto pValidator = CreateMosaicDurationValidator(BlockDuration(123));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, height);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "id " << notification.MosaicId;
		}
	}

	// region no duration change

	TEST(TEST_CLASS, SuccessWhenNonEternalMosaicIsKnownAndDeltaIsZero) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, 0);
		auto notification = CreateNotification(owner, properties);

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, owner, BlockDuration(123));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, notification);
	}

	TEST(TEST_CLASS, SuccessWhenEternalMosaicIsKnownAndDeltaIsZero) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, 0);
		auto notification = CreateNotification(owner, properties);

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, owner);

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, notification);
	}

	// endregion

	// region (new) unknown mosaic

	TEST(TEST_CLASS, SuccessWhenMosaicIsUnknownAndNotificationDurationDoesNotExceedMaxDuration) {
		// Arrange: create an empty cache
		auto owner = test::CreateRandomOwner();
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());

		// Assert: max duration is 123
		for (auto duration : { 1u, 70u, 123u }) {
			auto properties = test::CreateMosaicPropertiesFromValues(0, 0, duration);
			AssertValidationResult(ValidationResult::Success, cache, CreateNotification(owner, properties));
		}
	}

	TEST(TEST_CLASS, FailureWhenMosaicIsUnknownAndNotificationDurationExceedsMaxDuration) {
		// Arrange: create an empty cache
		auto owner = test::CreateRandomOwner();
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());

		// Assert: max duration is 123
		for (auto duration : { 124u, 999u }) {
			auto properties = test::CreateMosaicPropertiesFromValues(0, 0, duration);
			AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, CreateNotification(owner, properties));
		}
	}

	// endregion

	// region known mosaic

	TEST(TEST_CLASS, FailureWhenChangingDurationFromEternalToNonEternal) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, 123);
		auto notification = CreateNotification(owner, properties);

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, owner);

		// Assert:
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, notification);
	}

	TEST(TEST_CLASS, FailureWhenResultingDurationExceedsMaxDuration) {
		// Arrange:
		auto owner = test::CreateRandomOwner();

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, owner, BlockDuration(100));

		// Assert: max duration is 123
		for (auto duration : { 24u, 25u, 999u }) {
			auto properties = test::CreateMosaicPropertiesFromValues(0, 0, duration);
			AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, CreateNotification(owner, properties));
		}
	}

	TEST(TEST_CLASS, FailureWhenDurationOverflowHappens) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto properties = test::CreateMosaicPropertiesFromValues(0, 0, std::numeric_limits<uint64_t>::max() - 90);
		auto notification = CreateNotification(owner, properties);

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, owner, BlockDuration(100));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, notification);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsKnownAndNewDurationIsAcceptable_NonEternal) {
		// Arrange:
		auto owner = test::CreateRandomOwner();

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, owner, BlockDuration(100));

		// Assert:
		for (auto duration : { 1u, 22u, 23u }) {
			auto properties = test::CreateMosaicPropertiesFromValues(0, 0, duration);
			AssertValidationResult(ValidationResult::Success, cache, CreateNotification(owner, properties));
		}
	}

	// endregion
}}
