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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicDurationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicDuration, BlockDuration(123))

	namespace {
		constexpr MosaicId Default_Mosaic_Id = MosaicId(0x1234);

		model::MosaicDefinitionNotification CreateNotification(const Key& signer, BlockDuration duration) {
			auto properties = model::MosaicProperties::FromValues({ { 1, 2, duration.unwrap() } });
			return model::MosaicDefinitionNotification(signer, Default_Mosaic_Id, properties);
		}

		void AddMosaic(cache::CatapultCache& cache, const Key& owner, BlockDuration duration) {
			auto delta = cache.createDelta();
			test::AddMosaic(delta, Default_Mosaic_Id, Height(50), duration, owner);
			cache.commit(Height());
		}

		void AddEternalMosaic(cache::CatapultCache& cache, const Key& owner) {
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

	// region failure

	TEST(TEST_CLASS, FailureWhenChangingDurationFromEternalToNonEternal) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, BlockDuration(123));

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, signer);

		// Assert:
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, notification);
	}

	TEST(TEST_CLASS, FailureWhenChangingDurationFromNonEternalToEternal) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, BlockDuration());

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, signer, BlockDuration(123));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, notification);
	}

	TEST(TEST_CLASS, FailureWhenResultingDurationExceedsMaxDuration) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, signer, BlockDuration(100));

		// Assert: max duration is 123
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, CreateNotification(signer, BlockDuration(24)));
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, CreateNotification(signer, BlockDuration(25)));
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, CreateNotification(signer, BlockDuration(999)));
	}

	TEST(TEST_CLASS, FailureWhenOverflowHappens) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, BlockDuration(std::numeric_limits<uint64_t>::max() - 90));

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, signer, BlockDuration(100));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Invalid_Duration, cache, notification);
	}

	// endregion

	// region success

	TEST(TEST_CLASS, SuccessWhenMosaicIsUnknown) {
		// Arrange: although max duration is 123, it still should pass (MosaicPropertiesValidator checks for max duration)
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, BlockDuration(124));

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, notification);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsKnownAndNewDurationIsAcceptable_NonEternal) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, signer, BlockDuration(100));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, CreateNotification(signer, BlockDuration(1)));
		AssertValidationResult(ValidationResult::Success, cache, CreateNotification(signer, BlockDuration(22)));
		AssertValidationResult(ValidationResult::Success, cache, CreateNotification(signer, BlockDuration(23)));
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsKnownAndNewDurationIsAcceptable_Eternal) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();

		// - seed the cache
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, signer);

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, CreateNotification(signer, BlockDuration(0)));
	}

	// endregion
}}
