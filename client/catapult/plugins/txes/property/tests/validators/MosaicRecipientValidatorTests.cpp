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
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicRecipientValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicRecipient,)

	namespace {
		template<typename TOperationTraits>
		void PopulateCache(cache::CatapultCache& cache, const Address& accountAddress, const std::vector<MosaicId>& mosaicIds) {
			auto delta = cache.createDelta();
			auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
			propertyCacheDelta.insert(state::AccountProperties(accountAddress));
			auto& accountProperties = propertyCacheDelta.find(accountAddress).get();
			auto& accountProperty = accountProperties.property(model::PropertyType::MosaicId);
			for (auto mosaicId : mosaicIds)
				TOperationTraits::Add(accountProperty, state::ToVector(mosaicId));

			cache.commit(Height(1));
		}

		template<typename TOperationTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const Address& accountAddress,
				const std::vector<MosaicId>& mosaicIds,
				const Address& recipient,
				MosaicId mosaicId) {
			// Arrange:
			auto cache = test::PropertyCacheFactory::Create();
			PopulateCache<TOperationTraits>(cache, accountAddress, mosaicIds);
			auto pValidator = CreateMosaicRecipientValidator();
			auto sender = test::GenerateRandomData<Key_Size>();
			auto notification = model::BalanceTransferNotification(sender, recipient, mosaicId, Amount(123));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenRecipientIsKnownAndMosaicIdIsNotContainedInValues_Allow) {
		// Arrange:
		auto accountAddress = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		AssertValidationResult<test::AllowTraits>(
				Failure_Property_Mosaic_Transfer_Not_Allowed,
				accountAddress,
				test::GenerateRandomDataVector<MosaicId>(3),
				accountAddress,
				test::GenerateRandomValue<MosaicId>());
	}

	TEST(TEST_CLASS, FailureWhenRecipientIsKnownAndMosaicIdIsContainedInValues_Block) {
		// Arrange:
		auto accountAddress = test::GenerateRandomData<Address_Decoded_Size>();
		auto values = test::GenerateRandomDataVector<MosaicId>(3);

		// Act:
		AssertValidationResult<test::BlockTraits>(
				Failure_Property_Mosaic_Transfer_Not_Allowed,
				accountAddress,
				values,
				accountAddress,
				values[1]);
	}

	// endregion

	// region success

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(SuccessWhenRecipientIsNotKnown) {
		// Arrange:
		auto accountAddress = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		AssertValidationResult<TTraits>(
				ValidationResult::Success,
				accountAddress,
				test::GenerateRandomDataVector<MosaicId>(3),
				test::GenerateRandomData<Address_Decoded_Size>(),
				test::GenerateRandomValue<MosaicId>());
	}

	TRAITS_BASED_TEST(SuccessWhenRecipientIsKnownButPropertyHasNoValues) {
		// Arrange:
		auto accountAddress = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		AssertValidationResult<TTraits>(
				ValidationResult::Success,
				accountAddress,
				test::GenerateRandomDataVector<MosaicId>(0),
				accountAddress,
				test::GenerateRandomValue<MosaicId>());
	}

	namespace {
		template<typename TOperationTraits>
		void AssertSuccess(const std::vector<MosaicId>& mosaicIds, MosaicId transferredMosaicId) {
			// Arrange:
			auto accountAddress = test::GenerateRandomData<Address_Decoded_Size>();

			// Act:
			AssertValidationResult<TOperationTraits>(
					ValidationResult::Success,
					accountAddress,
					mosaicIds,
					accountAddress,
					transferredMosaicId);
		}
	}

	TEST(TEST_CLASS, SuccessWhenAllConditionsAreMet_Allow) {
		// Arrange:
		auto mosaicIds = test::GenerateRandomDataVector<MosaicId>(3);

		// Act:
		AssertSuccess<test::AllowTraits>(mosaicIds, mosaicIds[1]);
	}

	TEST(TEST_CLASS, SuccessWhenAllConditionsAreMet_Block) {
		// Act:
		AssertSuccess<test::BlockTraits>(test::GenerateRandomDataVector<MosaicId>(3), test::GenerateRandomValue<MosaicId>());
	}

	// endregion
}}
