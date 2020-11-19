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
#include "tests/test/MosaicRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicRestrictionRequiredValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicRestrictionRequired,)

	// region traits

	namespace {
		class TestContext {
		public:
			using NotificationType = model::MosaicRestrictionRequiredNotification;

		public:
			TestContext() : m_mosaicId(test::GenerateRandomValue<MosaicId>())
			{}

		public:
			NotificationType createNotification(uint64_t key) {
				return NotificationType(test::UnresolveXor(m_mosaicId), key);
			}

			void addRestrictionWithValueToCache(cache::MosaicRestrictionCacheDelta& restrictionCache, uint64_t key) {
				state::MosaicGlobalRestriction restriction(m_mosaicId);
				restriction.set(key, { MosaicId(), 246, model::MosaicRestrictionType::EQ });
				restrictionCache.insert(state::MosaicRestrictionEntry(restriction));
			}

		private:
			MosaicId m_mosaicId;
		};
	}

	// endregion

	// region tests

	TEST(TEST_CLASS, FailureWhenCacheDoesNotContainEntry) {
		// Arrange:
		TestContext context;

		auto pValidator = CreateMosaicRestrictionRequiredValidator();
		auto notification = context.createNotification(123);
		auto cache = test::MosaicRestrictionCacheFactory::Create();

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification, cache);

		// Assert:
		EXPECT_EQ(Failure_RestrictionMosaic_Unknown_Global_Restriction, result);
	}

	namespace {
		void RunCacheEntryTest(ValidationResult expectedResult, uint64_t notificationKey, uint64_t cacheKey) {
			// Arrange:
			TestContext context;

			auto pValidator = CreateMosaicRestrictionRequiredValidator();
			auto notification = context.createNotification(notificationKey);
			auto cache = test::MosaicRestrictionCacheFactory::Create();
			{
				auto delta = cache.createDelta();
				context.addRestrictionWithValueToCache(delta.sub<cache::MosaicRestrictionCache>(), cacheKey);
				cache.commit(Height());
			}

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenCacheContainsEntryButNotRule) {
		RunCacheEntryTest(Failure_RestrictionMosaic_Unknown_Global_Restriction, 123, 124);
	}

	TEST(TEST_CLASS, SuccessWhenCacheContainsEntryAndRule) {
		RunCacheEntryTest(ValidationResult::Success, 123, 123);
	}

	// endregion
}}
