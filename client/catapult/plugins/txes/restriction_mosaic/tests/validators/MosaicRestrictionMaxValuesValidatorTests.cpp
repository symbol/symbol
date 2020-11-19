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
#include "tests/test/MosaicRestrictionTestTraits.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicRestrictionMaxValuesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicGlobalRestrictionMaxValues, 10)

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicAddressRestrictionMaxValues, 10)

	// region traits

	namespace {
		enum class OperationType { Add, Modify, Delete };

		template<typename TTestTraits>
		class BasicTraits {
		public:
			using NotificationType = typename TTestTraits::NotificationType;

		public:
			NotificationType createNotification(uint64_t key, OperationType operation) {
				if (OperationType::Delete == operation)
					return m_traits.createDeleteNotification(key);
				else
					return m_traits.createAddNotification(key, 123);
			}

			void addRestrictionWithValuesToCache(cache::MosaicRestrictionCacheDelta& restrictionCache, size_t count) {
				std::vector<std::pair<uint64_t, uint64_t>> keyValuePairs;
				for (auto i = 0u; i < count; ++i)
					keyValuePairs.emplace_back(i, i);

				m_traits.addRestrictionWithValuesToCache(restrictionCache, keyValuePairs);
			}

		private:
			TTestTraits m_traits;
		};

		using GlobalTestTraits = test::MosaicGlobalRestrictionTestTraits<model::MosaicGlobalRestrictionModificationNewValueNotification>;
		using AddressTestTraits = test::MosaicAddressRestrictionTestTraits<
			model::MosaicAddressRestrictionModificationNewValueNotification>;

		class GlobalTraits : public BasicTraits<GlobalTestTraits> {
		public:
			static constexpr auto CreateValidator = CreateMosaicGlobalRestrictionMaxValuesValidator;
		};

		class AddressTraits : public BasicTraits<AddressTestTraits> {
		public:
			static constexpr auto CreateValidator = CreateMosaicAddressRestrictionMaxValuesValidator;
		};
	}

#define RESTRICTION_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(MosaicGlobalRestrictionMaxValuesValidatorTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GlobalTraits>(); } \
	TEST(MosaicAddressRestrictionMaxValuesValidatorTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region cache does not contain entry

	namespace {
		template<typename TTraits>
		void RunCacheDoesNotContainEntryTest(ValidationResult expectedResult, OperationType operationType) {
			// Arrange:
			TTraits traits;

			auto pValidator = TTraits::CreateValidator(10);
			auto notification = traits.createNotification(222, operationType);
			auto cache = test::MosaicRestrictionCacheFactory::Create();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	RESTRICTION_TYPE_BASED_TEST(SuccessWhenCacheDoesNotContainEntryAndNotificationIsAdd) {
		RunCacheDoesNotContainEntryTest<TTraits>(ValidationResult::Success, OperationType::Add);
	}

	RESTRICTION_TYPE_BASED_TEST(FailureWhenCacheDoesNotContainEntryAndNotificationIsDelete) {
		RunCacheDoesNotContainEntryTest<TTraits>(Failure_RestrictionMosaic_Cannot_Delete_Nonexistent_Restriction, OperationType::Delete);
	}

	// endregion

	// region cache contains entry but not rule

	namespace {
		template<typename TTraits>
		void RunCacheContainsEntryButNotRuleTest(ValidationResult expectedResult, OperationType operationType) {
			// Arrange:
			TTraits traits;

			auto pValidator = TTraits::CreateValidator(10);
			auto notification = traits.createNotification(222, operationType);
			auto cache = test::MosaicRestrictionCacheFactory::Create();
			{
				auto delta = cache.createDelta();
				traits.addRestrictionWithValuesToCache(delta.sub<cache::MosaicRestrictionCache>(), 3);
				cache.commit(Height());
			}

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	RESTRICTION_TYPE_BASED_TEST(SuccessWhenCacheContainsEntryButNotRuleAndNotificationIsAdd) {
		RunCacheContainsEntryButNotRuleTest<TTraits>(ValidationResult::Success, OperationType::Add);
	}

	RESTRICTION_TYPE_BASED_TEST(FailureWhenCacheContainsEntryButNotRuleAndNotificationIsDelete) {
		RunCacheContainsEntryButNotRuleTest<TTraits>(
				Failure_RestrictionMosaic_Cannot_Delete_Nonexistent_Restriction,
				OperationType::Delete);
	}

	// endregion

	// region max values

	namespace {
		template<typename TTraits>
		void RunCacheMaxValuesTest(
				ValidationResult expectedResult,
				uint8_t numInitialValues,
				uint8_t numMaxValues,
				OperationType operationType) {
			// Arrange:
			TTraits traits;

			uint64_t notificationRestrictionValue = operationType == OperationType::Add ? numMaxValues + 10 : numInitialValues - 2;
			auto pValidator = TTraits::CreateValidator(numMaxValues);
			auto notification = traits.createNotification(notificationRestrictionValue, operationType);
			auto cache = test::MosaicRestrictionCacheFactory::Create();
			{
				auto delta = cache.createDelta();
				traits.addRestrictionWithValuesToCache(delta.sub<cache::MosaicRestrictionCache>(), numInitialValues);
				cache.commit(Height());
			}

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	RESTRICTION_TYPE_BASED_TEST(SuccessWhenCacheEntryContainsOneLessThanMaxValuesAndNotificationIsAdd) {
		RunCacheMaxValuesTest<TTraits>(ValidationResult::Success, 9, 10, OperationType::Add);
	}

	RESTRICTION_TYPE_BASED_TEST(SuccessWhenCacheEntryContainsMaxValuesAndNotificationIsDelete) {
		RunCacheMaxValuesTest<TTraits>(ValidationResult::Success, 10, 10, OperationType::Delete);
	}

	RESTRICTION_TYPE_BASED_TEST(SuccessWhenCacheEntryContainsMaxValuesAndNotificationIsModify) {
		RunCacheMaxValuesTest<TTraits>(ValidationResult::Success, 10, 10, OperationType::Modify);
	}

	RESTRICTION_TYPE_BASED_TEST(FailureWhenCacheEntryContainsMaxValuesAndNotificationIsAdd) {
		RunCacheMaxValuesTest<TTraits>(Failure_RestrictionMosaic_Max_Restrictions_Exceeded, 10, 10, OperationType::Add);
	}

	// endregion
}}
