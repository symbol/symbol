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
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicRequiredValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RequiredMosaic,)

	namespace {
		constexpr auto Mosaic_Expiry_Height = Height(150);

		struct ResolvedMosaicTraits {
			static constexpr auto Default_Id = MosaicId(110);
		};

		struct UnresolvedMosaicTraits {
			// custom resolver doubles unresolved mosaic ids
			static constexpr auto Default_Id = UnresolvedMosaicId(110 ^ 0xFFFF'FFFF'FFFF'FFFF);
		};
	}

#define MOSAIC_ID_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Resolved) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ResolvedMosaicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Unresolved) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UnresolvedMosaicTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region zero property mask

	namespace {
		template<typename TMosaicId>
		void AssertValidationResult(
				ValidationResult expectedResult,
				TMosaicId affectedMosaicId,
				Height height,
				const model::ResolvableAddress& notificationOwner,
				const Address& artifactOwner) {
			// Arrange:
			auto pValidator = CreateRequiredMosaicValidator();

			// - create the notification
			model::MosaicRequiredNotification notification(notificationOwner, affectedMosaicId);

			// - create the validator context
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			auto delta = cache.createDelta();
			test::AddMosaic(delta, ResolvedMosaicTraits::Default_Id, Height(50), BlockDuration(100), artifactOwner);

			auto readOnlyCache = delta.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", id " << affectedMosaicId;
		}

		template<typename TMosaicId>
		void AssertValidationResult(ValidationResult expectedResult, TMosaicId affectedMosaicId, Height height) {
			auto owner = test::CreateRandomOwner();
			AssertValidationResult(expectedResult, affectedMosaicId, height, owner, owner);
		}
	}

	MOSAIC_ID_TRAITS_BASED_TEST(FailureWhenMosaicIsUnknown) {
		auto unknownMosaicId = TTraits::Default_Id + decltype(TTraits::Default_Id)(1);
		AssertValidationResult(Failure_Mosaic_Expired, unknownMosaicId, Height(100));
	}

	MOSAIC_ID_TRAITS_BASED_TEST(FailureWhenMosaicExpired) {
		AssertValidationResult(Failure_Mosaic_Expired, TTraits::Default_Id, Mosaic_Expiry_Height);
	}

	MOSAIC_ID_TRAITS_BASED_TEST(FailureWhenMosaicOwnerDoesNotMatch) {
		auto owner1 = test::CreateRandomOwner();
		auto owner2 = test::CreateRandomOwner();
		AssertValidationResult(Failure_Mosaic_Owner_Conflict, TTraits::Default_Id, Height(100), owner1, owner2);
	}

	MOSAIC_ID_TRAITS_BASED_TEST(SuccessWhenMosaicIsActiveAndOwnerMatches) {
		AssertValidationResult(ValidationResult::Success, TTraits::Default_Id, Height(100));
	}

	MOSAIC_ID_TRAITS_BASED_TEST(SuccessWhenMosaicIsActiveAndOwnerMatches_UnresolvedAddress) {
		auto owner = test::CreateRandomOwner();
		AssertValidationResult(ValidationResult::Success, TTraits::Default_Id, Height(100), test::UnresolveXor(owner), owner);
	}

	// endregion

	// region nonzero property mask

	namespace {
		template<typename TMosaicId>
		void AssertPropertyFlagMaskValidationResult(
				ValidationResult expectedResult,
				TMosaicId affectedMosaicId,
				uint8_t notificationPropertyFlagMask,
				uint8_t mosaicPropertyFlagMask) {
			// Arrange:
			auto pValidator = CreateRequiredMosaicValidator();

			// - create the notification
			auto owner = test::CreateRandomOwner();
			model::MosaicRequiredNotification notification(owner, affectedMosaicId, notificationPropertyFlagMask);

			// - create the validator context
			auto height = Height(50);
			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			auto delta = cache.createDelta();

			{
				// need to set custom property flags, so can't use regular helpers (e.g. test::AddMosaic)
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();

				model::MosaicProperties properties(static_cast<model::MosaicFlags>(mosaicPropertyFlagMask), 0, BlockDuration(100));
				auto definition = state::MosaicDefinition(height, owner, 1, properties);
				mosaicCacheDelta.insert(state::MosaicEntry(ResolvedMosaicTraits::Default_Id, definition));
			}

			auto readOnlyCache = delta.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "notificationPropertyFlagMask " << static_cast<uint16_t>(notificationPropertyFlagMask)
					<< "mosaicPropertyFlagMask " << static_cast<uint16_t>(mosaicPropertyFlagMask);
		}
	}

	MOSAIC_ID_TRAITS_BASED_TEST(FailureWhenPropertyFlagMaskDoesNotOverlap) {
		// Assert: 101, 010
		AssertPropertyFlagMaskValidationResult(Failure_Mosaic_Required_Property_Flag_Unset, TTraits::Default_Id, 0x05, 0x02);
	}

	MOSAIC_ID_TRAITS_BASED_TEST(FailureWhenPropertyFlagMaskPartiallyOverlaps) {
		// Assert: 101, 110
		AssertPropertyFlagMaskValidationResult(Failure_Mosaic_Required_Property_Flag_Unset, TTraits::Default_Id, 0x05, 0x06);
	}

	MOSAIC_ID_TRAITS_BASED_TEST(SuccessWhenPropertyFlagMaskIsExactMatch) {
		// Assert: 101, 101
		AssertPropertyFlagMaskValidationResult(ValidationResult::Success, TTraits::Default_Id, 0x05, 0x05);
	}

	MOSAIC_ID_TRAITS_BASED_TEST(SuccessWhenPropertyFlagMaskIsSubset) {
		// Assert: 101, 111
		AssertPropertyFlagMaskValidationResult(ValidationResult::Success, TTraits::Default_Id, 0x05, 0x07);
	}

	// endregion
}}
