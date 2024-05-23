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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS OperationRestrictionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(OperationRestriction, )

	namespace {
		constexpr auto Restriction_Flags = model::AccountRestrictionFlags::TransactionType | model::AccountRestrictionFlags::Outgoing;

		std::vector<uint16_t> DefaultRawTransactionTypes() {
			return { 0x4000, 0x4123, 0x4149 };
		}

		template<typename TOperationTraits>
		void PopulateCache(cache::CatapultCache& cache, const Address& address, const std::vector<uint16_t>& rawValues) {
			auto delta = cache.createDelta();
			auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
			restrictionCacheDelta.insert(state::AccountRestrictions(address));
			auto& restrictions = restrictionCacheDelta.find(address).get();
			auto& restriction = restrictions.restriction(Restriction_Flags);
			for (auto rawValue : rawValues)
				TOperationTraits::Add(restriction, state::ToVector(rawValue));

			cache.commit(Height(1));
		}

		template<typename TOperationTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const Address& accountAddress,
				const std::vector<uint16_t>& rawValues,
				const Address& sender,
				const model::EntityType& transactionType) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			PopulateCache<TOperationTraits>(cache, accountAddress, rawValues);
			auto pValidator = CreateOperationRestrictionValidator();
			auto notification = model::TransactionNotification(sender, Hash256(), transactionType, Timestamp(123));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenAccountIsKnownAndTransactionTypeIsNotContainedInValues_Allow) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();

		// Act:
		AssertValidationResult<test::AllowTraits>(
				Failure_RestrictionAccount_Operation_Type_Prohibited,
				sender,
				DefaultRawTransactionTypes(),
				sender,
				static_cast<model::EntityType>(0x4040));
	}

	TEST(TEST_CLASS, FailureWhenAccountIsKnownAndTransactionTypeIsContainedInValues_Block) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto values = DefaultRawTransactionTypes();

		// Act:
		AssertValidationResult<test::BlockTraits>(
				Failure_RestrictionAccount_Operation_Type_Prohibited,
				sender,
				values,
				sender,
				static_cast<model::EntityType>(values[1]));
	}

	// endregion

	// region success

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits>(); \
	} \
	template<typename TTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(SuccessWhenAccountIsNotKnown) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto other = test::GenerateRandomByteArray<Address>();
		auto values = DefaultRawTransactionTypes();

		// Act:
		AssertValidationResult<TTraits>(ValidationResult::Success, other, values, sender, static_cast<model::EntityType>(0x4444));
	}

	TRAITS_BASED_TEST(SuccessWhenAccountIsKnownButAccountRestrictionHasNoValues) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();

		// Act:
		AssertValidationResult<TTraits>(ValidationResult::Success, sender, {}, sender, static_cast<model::EntityType>(0x4444));
	}

	namespace {
		template<typename TOperationTraits>
		void AssertSuccess(const std::vector<uint16_t>& rawValues, uint16_t rawTransactionType) {
			// Arrange:
			auto sender = test::GenerateRandomByteArray<Address>();

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					sender,
					rawValues,
					sender,
					static_cast<model::EntityType>(rawTransactionType));
		}
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnownAndEntityTypeIsContainedInValues_Allow) {
		// Arrange:
		auto values = DefaultRawTransactionTypes();

		// Act:
		AssertSuccess<test::AllowTraits>(values, values[1]);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnownAndEntityTypeIsNotContainedInValues_Block) {
		// Act:
		AssertSuccess<test::BlockTraits>(DefaultRawTransactionTypes(), 0x4444);
	}

	// endregion
}}
