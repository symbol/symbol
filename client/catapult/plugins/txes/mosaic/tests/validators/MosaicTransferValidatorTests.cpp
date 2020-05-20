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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicTransferValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicTransfer, UnresolvedMosaicId())

	namespace {
		constexpr auto Currency_Mosaic_Id = UnresolvedMosaicId(2345);

		auto CreateCache() {
			return test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		}

		model::MosaicProperties CreateMosaicProperties(model::MosaicFlags flags) {
			return model::MosaicProperties(flags, 0, BlockDuration());
		}

		state::MosaicDefinition CreateMosaicDefinition(Height height, const Address& owner, model::MosaicFlags flags) {
			return state::MosaicDefinition(height, owner, 3, CreateMosaicProperties(flags));
		}

		state::MosaicEntry CreateMosaicEntry(MosaicId mosaicId, const Address& owner, model::MosaicFlags flags) {
			auto mosaicDefinition = CreateMosaicDefinition(Height(100), owner, flags);
			return state::MosaicEntry(mosaicId, mosaicDefinition);
		}

		void SeedCacheWithMosaic(cache::CatapultCache& cache, const state::MosaicEntry& mosaicEntry) {
			auto cacheDelta = cache.createDelta();
			auto& mosaicCacheDelta = cacheDelta.sub<cache::MosaicCache>();
			mosaicCacheDelta.insert(mosaicEntry);
			cache.commit(Height());
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const model::BalanceTransferNotification& notification) {
			// Arrange:
			auto pValidator = CreateMosaicTransferValidator(Currency_Mosaic_Id);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingCurrencyMosaicTransfer) {
		// Arrange:
		auto notification = model::BalanceTransferNotification(Address(), UnresolvedAddress(), Currency_Mosaic_Id, Amount(123));
		auto cache = CreateCache();

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, notification);
	}

	namespace {
		constexpr MosaicId Valid_Mosaic_Id(222);
		constexpr UnresolvedMosaicId Unresolved_Unknown_Mosaic_Id(444);

		auto CreateAndSeedCache(const Address& owner, model::MosaicFlags flags) {
			// Arrange:
			auto cache = CreateCache();
			auto validMosaicEntry = CreateMosaicEntry(Valid_Mosaic_Id, owner, flags);
			SeedCacheWithMosaic(cache, validMosaicEntry);

			auto cacheDelta = cache.createDelta();
			test::AddMosaicOwner(cacheDelta, Valid_Mosaic_Id, validMosaicEntry.definition().ownerAddress(), Amount());
			cache.commit(Height());

			return cache;
		}

		void AssertMosaicsTest(ValidationResult expectedResult, UnresolvedMosaicId mosaicId) {
			// Arrange:
			auto owner = test::GenerateRandomByteArray<Address>();
			auto notification = model::BalanceTransferNotification(owner, UnresolvedAddress(), mosaicId, Amount(123));
			auto cache = CreateAndSeedCache(owner, model::MosaicFlags::Transferable);

			// Assert:
			AssertValidationResult(expectedResult, cache, notification);
		}
	}

	TEST(TEST_CLASS, FailureWhenValidatingUnknownMosaic) {
		AssertMosaicsTest(Failure_Mosaic_Expired, Unresolved_Unknown_Mosaic_Id);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingKnownMosaic) {
		AssertMosaicsTest(ValidationResult::Success, test::UnresolveXor(Valid_Mosaic_Id));
	}

	namespace {
		enum : uint8_t {
			None = 0x00,
			Owner_Is_Sender = 0x01,
			Owner_Is_Recipient = 0x02
		};

		void AssertNonTransferableMosaicsTest(ValidationResult expectedResult, uint8_t notificationFlags) {
			// Arrange:
			auto owner = test::GenerateRandomByteArray<Address>();
			auto cache = CreateAndSeedCache(owner, model::MosaicFlags::None);

			// - notice that BalanceTransferNotification holds references to sender + recipient
			auto mosaicId = test::UnresolveXor(Valid_Mosaic_Id);
			auto notification = model::BalanceTransferNotification(Address(), UnresolvedAddress(), mosaicId, Amount(123));

			if (notificationFlags & Owner_Is_Sender)
				notification.Sender = owner;

			if (notificationFlags & Owner_Is_Recipient) {
				const auto& recipient = cache.createView().sub<cache::AccountStateCache>().find(owner).get().Address;
				notification.Recipient = test::UnresolveXor(recipient);
			}

			// Assert:
			AssertValidationResult(expectedResult, cache, notification);
		}
	}

	TEST(TEST_CLASS, FailureWhenMosaicIsNonTransferableAndOwnerIsNotParticipant) {
		AssertNonTransferableMosaicsTest(Failure_Mosaic_Non_Transferable, None);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerIsSender) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Sender);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerIsRecipient) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Recipient);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerSendsToSelf) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Recipient | Owner_Is_Sender);
	}
}}
