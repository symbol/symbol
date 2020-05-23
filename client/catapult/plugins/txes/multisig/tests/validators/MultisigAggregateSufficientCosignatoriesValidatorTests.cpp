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
#include "src/plugins/MultisigAccountModificationTransactionPlugin.h"
#include "catapult/model/Address.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigAggregateSufficientCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigAggregateSufficientCosignatories, model::TransactionRegistry())

	namespace {
		constexpr auto Failure_Result = Failure_Aggregate_Missing_Cosignatures;

		constexpr auto ToAddress = test::NetworkAddressConversions<>::ToAddress;
		constexpr auto ToAddresses = test::NetworkAddressConversions<>::ToAddresses;
		constexpr auto ToUnresolvedAddresses = test::NetworkAddressConversions<>::ToUnresolvedAddresses;

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const Key& signer,
				const model::EmbeddedTransaction& subTransaction,
				const std::vector<Key>& cosignatories) {
			// Arrange: setup cosignatures
			auto cosignatures = test::GenerateCosignaturesFromCosignatories(cosignatories);

			// - use a registry with mock and multilevel multisig transactions registered
			//   mock is used to test default behavior
			//   multilevel multisig is used to test transactions with custom approval requirements
			model::TransactionRegistry transactionRegistry;
			transactionRegistry.registerPlugin(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Not_Top_Level));
			transactionRegistry.registerPlugin(plugins::CreateMultisigAccountModificationTransactionPlugin());

			using Notification = model::AggregateEmbeddedTransactionNotification;
			Notification notification(signer, subTransaction, cosignatures.size(), cosignatures.data());
			auto pValidator = CreateMultisigAggregateSufficientCosignatoriesValidator(transactionRegistry);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		std::unique_ptr<model::EmbeddedTransaction> CreateEmbeddedTransaction(const Key& signer) {
			auto pTransaction = std::make_unique<model::EmbeddedTransaction>();
			pTransaction->Type = mocks::MockTransaction::Entity_Type;
			pTransaction->SignerPublicKey = signer;
			return pTransaction;
		}
	}

	// region unknown account / known non-multisig account

	namespace {
		struct UnknownAccountTraits {
			static auto CreateCache(const Key&) {
				// return an empty cache
				return test::MultisigCacheFactory::Create();
			}
		};

		struct CosignatoryAccountTraits {
			static auto CreateCache(const Key& aggregateSigner) {
				auto cache = test::MultisigCacheFactory::Create();
				auto cacheDelta = cache.createDelta();

				// make the aggregate signer a cosignatory of a different account
				test::MakeMultisig(cacheDelta, test::GenerateRandomByteArray<Address>(), { ToAddress(aggregateSigner) });

				cache.commit(Height());
				return cache;
			}
		};

		template<typename TTraits>
		static void AssertValidationResult(
				ValidationResult expectedResult,
				const Key& embeddedSigner,
				const Key& aggregateSigner,
				const std::vector<Key>& cosignatories) {
			// Arrange:
			auto cache = TTraits::CreateCache(embeddedSigner);
			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// Assert:
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosignatories);
		}
	}

#define NON_MULTISIG_TRAITS_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_UnknownAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UnknownAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_CosignatoryAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CosignatoryAccountTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	NON_MULTISIG_TRAITS_TEST(SufficientWhenEmbeddedTransactionSignerIsAggregateSigner) {
		// Arrange:
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(2);

		// Assert: aggregate signer (implicit cosignatory) is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, aggregateSigner, aggregateSigner, cosignatories);
	}

	NON_MULTISIG_TRAITS_TEST(SufficientWhenEmbeddedTransactionSignerIsCosignatory) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(3);
		cosignatories[1] = embeddedSigner;

		// Assert: one cosignatory is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, embeddedSigner, aggregateSigner, cosignatories);
	}

	NON_MULTISIG_TRAITS_TEST(InsufficientWhenTransactionSignerIsNeitherAggregateSignerNorCosignatory) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(2);

		// Assert: embedded transaction signer is neither aggregate signer nor cosignatory
		AssertValidationResult<TTraits>(Failure_Result, embeddedSigner, aggregateSigner, cosignatories);
	}

	// endregion

	// region single level multisig

	namespace {
		auto CreateCacheWithSingleLevelMultisig(
				const Key& embeddedSigner,
				const std::vector<Key>& cosignatories,
				uint32_t minApproval = 3,
				uint32_t minRemoval = 4) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// make a (3-4-X default) multisig
			test::MakeMultisig(cacheDelta, ToAddress(embeddedSigner), ToAddresses(cosignatories), minApproval, minRemoval);

			cache.commit(Height());
			return cache;
		}

		template<typename TSelectCosignatories>
		void AssertBasicMultisigResult(ValidationResult expectedResult, TSelectCosignatories selectCosignatories) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(4);

			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// - create the cache making the embedded signer a 3-4-4 multisig
			auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories);

			// Assert: 3 cosignatories are required for approval
			auto selectedCosignatories = selectCosignatories(cosignatories);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, selectedCosignatories);
		}
	}

	TEST(TEST_CLASS, SufficientWhenMultisigEmbeddedTransactionSignerHasMinApprovers) {
		// Assert: 3 == 3
		AssertBasicMultisigResult(ValidationResult::Success, [](const auto& cosignatories) {
			return std::vector<Key>{ cosignatories[0], cosignatories[2], cosignatories[3] };
		});
	}

	TEST(TEST_CLASS, SufficientWhenMultisigEmbeddedTransactionSignerHasGreaterThanMinApprovers) {
		// Assert: 4 > 3
		AssertBasicMultisigResult(ValidationResult::Success, [](const auto& cosignatories) {
			return cosignatories;
		});
	}

	TEST(TEST_CLASS, InsufficientWhenMultisigEmbeddedTransactionSignerHasLessThanMinApprovers) {
		// Assert: 2 < 3
		AssertBasicMultisigResult(Failure_Result, [](const auto& cosignatories) {
			return std::vector<Key>{ cosignatories[0], cosignatories[2] };
		});
	}

	TEST(TEST_CLASS, SufficientWhenMultisigEmbeddedTransactionSignerHasMinApproversIncludingAggregateSigner) {
		// Arrange: include the aggregate signer as a cosignatory
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(4);
		cosignatories.push_back(aggregateSigner);

		auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

		// - create the cache making the embedded signer a 3-4-5 multisig
		auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories);

		// Assert: 1 (implicit) + 2 (explicit) == 3
		auto selectedCosignatories = std::vector<Key>{ cosignatories[2], cosignatories[1] };
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, selectedCosignatories);
	}

	// endregion

	// region multilevel multisig

	namespace {
		auto CreateCacheWithMultilevelMultisig(
				const Key& embeddedSigner,
				const std::vector<Key>& cosignatories,
				const std::vector<Key>& secondLevelCosignatories) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// make a 2-3-X multisig
			test::MakeMultisig(cacheDelta, ToAddress(embeddedSigner), ToAddresses(cosignatories), 2, 3);

			// make a 3-4-X multisig
			test::MakeMultisig(cacheDelta, ToAddress(cosignatories[1]), ToAddresses(secondLevelCosignatories), 3, 4);

			cache.commit(Height());
			return cache;
		}

		template<typename TGetCosignatories>
		void AssertBasicMultilevelMultisigResult(ValidationResult expectedResult, TGetCosignatories getCosignatories) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(3);
			auto secondLevelCosignatories = test::GenerateRandomDataVector<Key>(4);

			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// - create the cache making the embedded signer a 2-3-3 multisig where the second cosignatory is a 3-4-4 multisig
			auto cache = CreateCacheWithMultilevelMultisig(embeddedSigner, cosignatories, secondLevelCosignatories);

			// Assert: 2 first-level and 3 second-level cosignatories are required for approval
			auto selectedCosignatories = getCosignatories(secondLevelCosignatories);
			selectedCosignatories.push_back(cosignatories[1]);
			selectedCosignatories.push_back(cosignatories[2]);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, selectedCosignatories);
		}
	}

	TEST(TEST_CLASS, SufficientWhenMultilevelMultisigEmbeddedTransactionSignerHasMinApprovers) {
		// Assert: 3 == 3
		AssertBasicMultilevelMultisigResult(ValidationResult::Success, [](const auto& cosignatories) {
			return std::vector<Key>{ cosignatories[0], cosignatories[2], cosignatories[3] };
		});
	}

	TEST(TEST_CLASS, SufficientWhenMultilevelMultisigEmbeddedTransactionSignerHasGreaterThanMinApprovers) {
		// Assert: 4 > 3
		AssertBasicMultilevelMultisigResult(ValidationResult::Success, [](const auto& cosignatories) {
			return cosignatories;
		});
	}

	TEST(TEST_CLASS, InsufficientWhenMultilevelMultisigEmbeddedTransactionSignerHasLessThanMinApprovers) {
		// Assert: 2 < 3
		AssertBasicMultilevelMultisigResult(Failure_Result, [](const auto& cosignatories) {
			return std::vector<Key>{ cosignatories[0], cosignatories[2] };
		});
	}

	TEST(TEST_CLASS, SuccessWhenMultisigTransactionSignerHasMinApproversSharedAcrossLevels) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(3);
		auto secondLevelCosignatories = test::GenerateRandomDataVector<Key>(4);
		secondLevelCosignatories.push_back(cosignatories[2]);

		auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

		// - create the cache making the embedded signer a 2-3-3 multisig where the second cosignatory is a 3-4-5 multisig
		auto cache = CreateCacheWithMultilevelMultisig(embeddedSigner, cosignatories, secondLevelCosignatories);

		// Assert: 2 first-level and 3 second-level cosignatories are required for approval
		//         (notice that cosignatories[2] cosigns both levels)
		auto selectedCosignatories = std::vector<Key>{
			cosignatories[1], cosignatories[2],
			secondLevelCosignatories[0], secondLevelCosignatories[1]
		};
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, selectedCosignatories);
	}

	// endregion

	// region multisig account modification handling

	namespace {
		void AddAll(std::vector<Key>& allCosignatories, const std::vector<Key>& cosignatories) {
			allCosignatories.insert(allCosignatories.cend(), cosignatories.cbegin(), cosignatories.cend());
		}

		void AssertMinApprovalLimit(
				uint32_t expectedLimit,
				uint8_t numAdditions,
				uint8_t numDeletions,
				uint32_t minApproval,
				uint32_t minRemoval) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(expectedLimit);

			auto publicKeyAdditions = test::GenerateRandomDataVector<Key>(numAdditions);
			auto publicKeyDeletions = test::GenerateRandomDataVector<Key>(numDeletions);
			auto pSubTransaction = test::CreateMultisigAccountModificationTransaction(
					embeddedSigner,
					ToUnresolvedAddresses(publicKeyAdditions),
					ToUnresolvedAddresses(publicKeyDeletions));

			// - create the cache making the embedded signer a single level multisig
			auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories, minApproval, minRemoval);

			// Assert:
			CATAPULT_LOG(debug) << "running test with " << expectedLimit - 1 << " cosignatories (insufficient)";
			auto insufficientCosignatories = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit - 1);
			AddAll(insufficientCosignatories, publicKeyAdditions);
			AssertValidationResult(Failure_Result, cache, aggregateSigner, *pSubTransaction, insufficientCosignatories);

			CATAPULT_LOG(debug) << "running test with " << expectedLimit << " cosignatories (sufficient)";
			auto sufficientCosignatories = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit);
			AddAll(sufficientCosignatories, publicKeyAdditions);
			AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosignatories);
		}
	}

	TEST(TEST_CLASS, SingleAddModificationRequiresMinApproval) {
		AssertMinApprovalLimit(3, 1, 0, 3, 4);
	}

	TEST(TEST_CLASS, MultipleAddModificationsRequireMinApproval) {
		AssertMinApprovalLimit(3, 3, 0, 3, 4);
	}

	TEST(TEST_CLASS, SingleDelModificationRequiresMinRemoval) {
		AssertMinApprovalLimit(4, 0, 1, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinRemovalWhenMinRemovalIsGreater) {
		AssertMinApprovalLimit(4, 2, 1, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinApprovalWhenMinApprovalIsGreater) {
		AssertMinApprovalLimit(4, 2, 1, 4, 3);
	}

	TEST(TEST_CLASS, NonCosignatoryModificationRequiresMinApproval) {
		AssertMinApprovalLimit(3, 0, 0, 3, 4);
	}

	TEST(TEST_CLASS, MinRemovalLimitIsAppliedAcrossMultipleLevels) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(3);
		auto secondLevelCosignatories = test::GenerateRandomDataVector<Key>(4);
		auto base23Cosignatories = std::vector<Key>{
			cosignatories[0], cosignatories[1],
			secondLevelCosignatories[0], secondLevelCosignatories[1], secondLevelCosignatories[2]
		};

		auto pSubTransaction = test::CreateMultisigAccountModificationTransaction(embeddedSigner, 0, 1);

		// - create the cache making the embedded signer a 2-3-3 multisig where the second cosignatory is a 3-4-4 multisig
		auto cache = CreateCacheWithMultilevelMultisig(embeddedSigner, cosignatories, secondLevelCosignatories);

		// Assert: 3 first-level and 4 second-level cosignatories are required for approval
		CATAPULT_LOG(debug) << "running test with 3 + 3 cosignatories (insufficient)";
		auto insufficient33Cosignatories = base23Cosignatories;
		insufficient33Cosignatories.push_back(cosignatories[2]);
		AssertValidationResult(Failure_Result, cache, aggregateSigner, *pSubTransaction, insufficient33Cosignatories);

		CATAPULT_LOG(debug) << "running test with 2 + 4 cosignatories (insufficient)";
		auto insufficient24Cosignatories = base23Cosignatories;
		insufficient24Cosignatories.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(Failure_Result, cache, aggregateSigner, *pSubTransaction, insufficient24Cosignatories);

		CATAPULT_LOG(debug) << "running test with 3 + 4 cosignatories (sufficient)";
		auto sufficientCosignatories = base23Cosignatories;
		sufficientCosignatories.push_back(cosignatories[2]);
		sufficientCosignatories.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosignatories);
	}

	// endregion

	// region cosignatory approval - utils

	namespace {
		enum class AccountPolicy { Regular, Multisig };

		void AddSingleLevelMultisig(cache::CatapultCache& cache, const Key& multisig, const std::vector<Key>& cosignatories) {
			auto cacheDelta = cache.createDelta();

			// make a (3-3-X default) multisig
			test::MakeMultisig(cacheDelta, ToAddress(multisig), ToAddresses(cosignatories), 3, 3);

			cache.commit(Height());
		}

		template<typename TMergeKeys>
		void AssertCosignatoriesMustApproveTransaction(
				ValidationResult expectedResult,
				uint8_t numAdditions,
				uint8_t numDeletions,
				const std::vector<AccountPolicy>& accountPolicies,
				TMergeKeys mergeKeys) {
			// Sanity:
			ASSERT_EQ(numAdditions, accountPolicies.size());

			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto embeddedSignerCosignatories = test::GenerateRandomDataVector<Key>(3);

			auto publicKeyAdditions = test::GenerateRandomDataVector<Key>(numAdditions);
			auto publicKeyDeletions = test::GenerateRandomDataVector<Key>(numDeletions);
			auto pSubTransaction = test::CreateMultisigAccountModificationTransaction(
					embeddedSigner,
					ToUnresolvedAddresses(publicKeyAdditions),
					ToUnresolvedAddresses(publicKeyDeletions));

			// - create the cache making the embedded signer a single level multisig
			auto cache = test::MultisigCacheFactory::Create();
			AddSingleLevelMultisig(cache, embeddedSigner, embeddedSignerCosignatories);

			// - make added cosignatories single level multisig according to the multisig policies
			std::vector<Key> requiredCosignatories;
			for (auto i = 0u; i < pSubTransaction->AddressAdditionsCount; ++i) {
				const auto& cosignatoryPublicKey = publicKeyAdditions[i];
				if (AccountPolicy::Multisig == accountPolicies[i]) {
					// - cosignatoryPublicKey is not a required cosignatory because it is a multisig account
					auto cosignatoryCosignatories = test::GenerateRandomDataVector<Key>(3);
					AddSingleLevelMultisig(cache, cosignatoryPublicKey, cosignatoryCosignatories);
					AddAll(requiredCosignatories, cosignatoryCosignatories);
				} else {
					requiredCosignatories.push_back(cosignatoryPublicKey);
				}
			}

			// Assert:
			auto cosignatories = mergeKeys(embeddedSignerCosignatories, requiredCosignatories);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosignatories);
		}

		template<typename TTraits>
		void AssertCosignatoriesMustApproveTransaction(const std::vector<AccountPolicy>& accountPolicies) {
			// Assert:
			AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, 3, 0, accountPolicies, TTraits::MergeKeys);
		}

		struct ValidationFailureTraits {
			static constexpr auto ExpectedResult = Failure_Result;

			static auto MergeKeys(const std::vector<Key>& embeddedSignerCosignatories, const std::vector<Key>&) {
				return embeddedSignerCosignatories;
			}
		};

		struct ValidationSuccessTraits {
			static constexpr auto ExpectedResult = ValidationResult::Success;

			static auto MergeKeys(const std::vector<Key>& embeddedSignerCosignatories, const std::vector<Key>& requiredCosignatories) {
				auto cosignatories = std::vector<Key>(embeddedSignerCosignatories.cbegin(), embeddedSignerCosignatories.cend());
				AddAll(cosignatories, requiredCosignatories);
				return cosignatories;
			}
		};
	}

	// endregion

	// region cosignatory approval - shared

#define COSIGNATORY_APPROVAL_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Failure) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationFailureTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Success) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationSuccessTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_SingleAdd_NotMultisig) {
		AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, 1, 0, { AccountPolicy::Regular }, TTraits::MergeKeys);
	}

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_SingleAdd_Multisig) {
		AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, 1, 0, { AccountPolicy::Multisig }, TTraits::MergeKeys);
	}

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_NoneMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Regular, AccountPolicy::Regular, AccountPolicy::Regular });
	}

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_SomeMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Multisig, AccountPolicy::Regular, AccountPolicy::Multisig });
	}

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_AllMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Multisig, AccountPolicy::Multisig, AccountPolicy::Multisig });
	}

	COSIGNATORY_APPROVAL_TEST(CosignatoryApproval_AddsAndDelete_SomeMultisig) {
		AssertCosignatoriesMustApproveTransaction(
				TTraits::ExpectedResult,
				2,
				1,
				{ AccountPolicy::Multisig, AccountPolicy::Regular },
				TTraits::MergeKeys);
	}

	// endregion

	// region cosignatory approval - failure

	TEST(TEST_CLASS, InsuffientWhenOnlySomeCosignatoriesApprove) {
		AssertCosignatoriesMustApproveTransaction(
				Failure_Result,
				2,
				1,
				{ AccountPolicy::Multisig, AccountPolicy::Regular },
				[](const auto& embeddedSignerCosignatories, const auto& requiredCosignatories) {
					auto cosignatories = ValidationSuccessTraits::MergeKeys(embeddedSignerCosignatories, requiredCosignatories);
					cosignatories.erase(--cosignatories.cend());
					return cosignatories;
				});
	}

	// endregion

	// region cosignatory approval - success

	TEST(TEST_CLASS, SufficientWhenOnlyDeleteModification) {
		constexpr auto Success = ValidationResult::Success;
		AssertCosignatoriesMustApproveTransaction(Success, 0, 1, {}, [](const auto& cosignatories, const auto&) {
			return cosignatories;
		});
	}

	// endregion
}}
