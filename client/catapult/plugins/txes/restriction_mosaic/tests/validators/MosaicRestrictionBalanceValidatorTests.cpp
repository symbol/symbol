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
#include "catapult/model/Address.h"
#include "tests/test/MosaicRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicRestrictionBalanceValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicRestrictionBalanceTransfer,)

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicRestrictionBalanceDebit,)

	// region traits

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(18);

		struct TransferSenderTraits {
			static constexpr auto CreateValidator = CreateMosaicRestrictionBalanceTransferValidator;

			static auto CreateNotification(const Key& publicKey1, const Key& publicKey2) {
				auto recipient = test::UnresolveXor(model::PublicKeyToAddress(publicKey2, Network_Identifier));
				return model::BalanceTransferNotification(publicKey1, recipient, UnresolvedMosaicId(), Amount());
			}
		};

		struct TransferRecipientTraits {
			static constexpr auto CreateValidator = CreateMosaicRestrictionBalanceTransferValidator;

			static auto CreateNotification(const Key& publicKey1, const Key& publicKey2) {
				auto recipient = test::UnresolveXor(model::PublicKeyToAddress(publicKey1, Network_Identifier));
				return model::BalanceTransferNotification(publicKey2, recipient, UnresolvedMosaicId(), Amount());
			}
		};

		struct DebitSenderTraits {
			static constexpr auto CreateValidator = CreateMosaicRestrictionBalanceDebitValidator;

			static auto CreateNotification(const Key& publicKey1, const Key&) {
				return model::BalanceDebitNotification(publicKey1, UnresolvedMosaicId(), Amount());
			}
		};
	}

#define ACCOUNT_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(MosaicRestrictionBalanceTransferValidatorTests, TEST_NAME##_Sender) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferSenderTraits>(); \
	} \
	TEST(MosaicRestrictionBalanceTransferValidatorTests, TEST_NAME##_Recipient) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferRecipientTraits>(); \
	} \
	TEST(MosaicRestrictionBalanceDebitValidatorTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DebitSenderTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region tests

	namespace {
		static constexpr auto Mosaic_Id_Invalid_Rule = MosaicId(222);
		static constexpr auto Mosaic_Id_Valid_Rule = MosaicId(111);
		static constexpr auto Mosaic_Id_No_Rules = MosaicId(333);

		void SeedCacheForTests(
				cache::MosaicRestrictionCacheDelta& delta,
				const std::vector<Key>& validPublicKeys,
				const std::vector<Key>& invalidPublicKeys) {
			// Arrange: invalid global restriction (self referential)
			auto restriction1 = state::MosaicGlobalRestriction(Mosaic_Id_Invalid_Rule);
			restriction1.set(200, { Mosaic_Id_Invalid_Rule, 777, model::MosaicRestrictionType::NE });
			delta.insert(state::MosaicRestrictionEntry(restriction1));

			// - valid global restriction (EQ 888)
			auto restriction2 = state::MosaicGlobalRestriction(Mosaic_Id_Valid_Rule);
			restriction2.set(200, { MosaicId(), 888, model::MosaicRestrictionType::EQ });
			delta.insert(state::MosaicRestrictionEntry(restriction2));

			// - valid addresses (EQ 888)
			for (const auto& publicKey : validPublicKeys) {
				auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
				auto restriction = state::MosaicAddressRestriction(Mosaic_Id_Valid_Rule, address);
				restriction.set(200, 888);
				delta.insert(state::MosaicRestrictionEntry(restriction));
			}

			// - invalid addresses (NE 888)
			for (const auto& publicKey : invalidPublicKeys) {
				auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
				auto restriction = state::MosaicAddressRestriction(Mosaic_Id_Valid_Rule, address);
				restriction.set(200, 789);
				delta.insert(state::MosaicRestrictionEntry(restriction));
			}
		}

		template<typename TTraits, typename TCacheSeeder>
		void RunTest(ValidationResult expectedResult, MosaicId mosaicId, TCacheSeeder seeder) {
			// Arrange:
			auto pValidator = TTraits::CreateValidator();

			auto publicKey1 = test::GenerateRandomByteArray<Key>();
			auto publicKey2 = test::GenerateRandomByteArray<Key>();
			auto notification = TTraits::CreateNotification(publicKey1, publicKey2);
			notification.MosaicId = test::UnresolveXor(mosaicId);
			notification.Amount = Amount(100);

			auto cache = test::MosaicRestrictionCacheFactory::Create(Network_Identifier);
			{
				auto delta = cache.createDelta();
				seeder(delta.sub<cache::MosaicRestrictionCache>(), publicKey1, publicKey2);
				cache.commit(Height());
			}

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	ACCOUNT_BASED_TEST(SuccessWhenNoMosaicRulesAreConfigured) {
		auto expectedResult = ValidationResult::Success;
		RunTest<TTraits>(expectedResult, Mosaic_Id_No_Rules, [](auto& cache, const auto&, const auto&) {
			SeedCacheForTests(cache, {}, {});
		});
	}

	ACCOUNT_BASED_TEST(FailureWhenInvalidMosaicRulesAreConfigured) {
		auto expectedResult = Failure_RestrictionMosaic_Global_Restriction_Invalid;
		RunTest<TTraits>(expectedResult, Mosaic_Id_Invalid_Rule, [](auto& cache, const auto& publicKey1, const auto& publicKey2) {
			SeedCacheForTests(cache, { publicKey1, publicKey2 }, {});
		});
	}

	ACCOUNT_BASED_TEST(FailureWhenValidMosaicRulesAreConfiguredButOneAccountIsUnauthorizedDueToInvalidValues) {
		auto expectedResult = Failure_RestrictionMosaic_Account_Unauthorized;
		RunTest<TTraits>(expectedResult, Mosaic_Id_Valid_Rule, [](auto& cache, const auto& publicKey1, const auto& publicKey2) {
			SeedCacheForTests(cache, { publicKey2 }, { publicKey1 });
		});
	}

	ACCOUNT_BASED_TEST(FailureWhenValidMosaicRulesAreConfiguredButOneAccountIsUnauthorizedDueToUnsetValues) {
		auto expectedResult = Failure_RestrictionMosaic_Account_Unauthorized;
		RunTest<TTraits>(expectedResult, Mosaic_Id_Valid_Rule, [](auto& cache, const auto&, const auto& publicKey2) {
			SeedCacheForTests(cache, { publicKey2 }, {});
		});
	}

	ACCOUNT_BASED_TEST(SuccessWhenValidMosaicRulesAreConfiguredAndAllAccountsAreAuthorized) {
		auto expectedResult = ValidationResult::Success;
		RunTest<TTraits>(expectedResult, Mosaic_Id_Valid_Rule, [](auto& cache, const auto& publicKey1, const auto& publicKey2) {
			SeedCacheForTests(cache, { publicKey1, publicKey2 }, {});
		});
	}

	// endregion
}}
