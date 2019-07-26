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
#include "src/plugins/ModifyMultisigAccountTransactionPlugin.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigAggregateSufficientCosignersValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigAggregateSufficientCosigners, model::TransactionRegistry())

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const Key& signer,
				const model::EmbeddedTransaction& subTransaction,
				const std::vector<Key>& cosigners) {
			// Arrange: setup cosignatures
			auto cosignatures = test::GenerateCosignaturesFromCosigners(cosigners);

			// - use a registry with mock and multilevel multisig transactions registered
			//   mock is used to test default behavior
			//   multilevel multisig is used to test transactions with custom approval requirements
			model::TransactionRegistry transactionRegistry;
			transactionRegistry.registerPlugin(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Not_Top_Level));
			transactionRegistry.registerPlugin(plugins::CreateModifyMultisigAccountTransactionPlugin());

			using Notification = model::AggregateEmbeddedTransactionNotification;
			Notification notification(signer, subTransaction, cosignatures.size(), cosignatures.data());
			auto pValidator = CreateMultisigAggregateSufficientCosignersValidator(transactionRegistry);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		std::unique_ptr<model::EmbeddedTransaction> CreateEmbeddedTransaction(const Key& signer) {
			auto pTransaction = std::make_unique<model::EmbeddedTransaction>();
			pTransaction->Type = mocks::MockTransaction::Entity_Type;
			pTransaction->Signer = signer;
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

				// make the aggregate signer a cosigner of a different account
				test::MakeMultisig(cacheDelta, test::GenerateRandomByteArray<Key>(), { aggregateSigner });

				cache.commit(Height());
				return cache;
			}
		};

		template<typename TTraits>
		static void AssertValidationResult(
				ValidationResult expectedResult,
				const Key& embeddedSigner,
				const Key& aggregateSigner,
				const std::vector<Key>& cosigners) {
			// Arrange:
			auto cache = TTraits::CreateCache(embeddedSigner);
			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// Assert:
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosigners);
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
		auto cosigners = test::GenerateRandomDataVector<Key>(2);

		// Assert: aggregate signer (implicit cosigner) is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, aggregateSigner, aggregateSigner, cosigners);
	}

	NON_MULTISIG_TRAITS_TEST(SufficientWhenEmbeddedTransactionSignerIsCosigner) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosigners = std::vector<Key>{ test::GenerateRandomByteArray<Key>(), embeddedSigner, test::GenerateRandomByteArray<Key>() };

		// Assert: one cosigner is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, embeddedSigner, aggregateSigner, cosigners);
	}

	NON_MULTISIG_TRAITS_TEST(InsufficientWhenTransactionSignerIsNeitherAggregateSignerNorCosigner) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(2);

		// Assert: embedded transaction signer is neither aggregate signer nor cosigner
		AssertValidationResult<TTraits>(Failure_Aggregate_Missing_Cosigners, embeddedSigner, aggregateSigner, cosigners);
	}

	// endregion

	// region single level multisig

	namespace {
		auto CreateCacheWithSingleLevelMultisig(
				const Key& embeddedSigner,
				const std::vector<Key>& cosignatories,
				uint8_t minApproval = 3,
				uint8_t minRemoval = 4) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			test::MakeMultisig(cacheDelta, embeddedSigner, cosignatories, minApproval, minRemoval); // make a (3-4-X default) multisig

			cache.commit(Height());
			return cache;
		}

		template<typename TGetCosigners>
		void AssertBasicMultisigResult(ValidationResult expectedResult, TGetCosigners getCosigners) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(4);

			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// - create the cache making the embedded signer a 3-4-4 multisig
			auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories);

			// Assert: 3 cosigners are required for approval
			auto cosigners = getCosigners(cosignatories);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosigners);
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
		AssertBasicMultisigResult(Failure_Aggregate_Missing_Cosigners, [](const auto& cosignatories) {
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
		auto cosigners = std::vector<Key>{ cosignatories[2], cosignatories[1] };
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, cosigners);
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

			test::MakeMultisig(cacheDelta, embeddedSigner, cosignatories, 2, 3); // make a 2-3-X multisig
			test::MakeMultisig(cacheDelta, cosignatories[1], secondLevelCosignatories, 3, 4); // make a 3-4-X multisig

			cache.commit(Height());
			return cache;
		}

		template<typename TGetCosigners>
		void AssertBasicMultilevelMultisigResult(ValidationResult expectedResult, TGetCosigners getCosigners) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(3);
			auto secondLevelCosignatories = test::GenerateRandomDataVector<Key>(4);

			auto pSubTransaction = CreateEmbeddedTransaction(embeddedSigner);

			// - create the cache making the embedded signer a 2-3-3 multisig where the second cosignatory is a 3-4-4 multisig
			auto cache = CreateCacheWithMultilevelMultisig(embeddedSigner, cosignatories, secondLevelCosignatories);

			// Assert: 2 first-level and 3 second-level cosigners are required for approval
			auto cosigners = getCosigners(secondLevelCosignatories);
			cosigners.push_back(cosignatories[1]);
			cosigners.push_back(cosignatories[2]);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosigners);
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
		AssertBasicMultilevelMultisigResult(Failure_Aggregate_Missing_Cosigners, [](const auto& cosignatories) {
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

		// Assert: 2 first-level and 3 second-level cosigners are required for approval (notice that cosignatories[2] cosigns both levels)
		auto cosigners = std::vector<Key>{
			cosignatories[1], cosignatories[2],
			secondLevelCosignatories[0], secondLevelCosignatories[1]
		};
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, cosigners);
	}

	// endregion

	// region multisig modify account handling

	namespace {
		constexpr auto Add = model::CosignatoryModificationType::Add;
		constexpr auto Del = model::CosignatoryModificationType::Del;

		void AddRequiredCosignersKeys(std::vector<Key>& cosigners, const model::EmbeddedModifyMultisigAccountTransaction& transaction) {
			auto numModifications = transaction.ModificationsCount;
			auto* pModification = transaction.ModificationsPtr();
			for (auto i = 0u; i < numModifications; ++i) {
				if (model::CosignatoryModificationType::Add == pModification->ModificationType)
					cosigners.push_back(pModification->CosignatoryPublicKey);

				++pModification;
			}
		}

		void AssertMinApprovalLimit(
				uint32_t expectedLimit,
				const std::vector<model::CosignatoryModificationType>& modificationTypes,
				uint8_t minApproval,
				uint8_t minRemoval) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(expectedLimit);

			auto pSubTransaction = test::CreateModifyMultisigAccountTransaction(embeddedSigner, modificationTypes);

			// - create the cache making the embedded signer a single level multisig
			auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories, minApproval, minRemoval);

			// Assert:
			CATAPULT_LOG(debug) << "running test with " << expectedLimit - 1 << " cosigners (insufficient)";
			auto insufficientCosigners = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit - 1);
			AddRequiredCosignersKeys(insufficientCosigners, *pSubTransaction);
			AssertValidationResult(Failure_Aggregate_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficientCosigners);

			CATAPULT_LOG(debug) << "running test with " << expectedLimit << " cosigners (sufficient)";
			auto sufficientCosigners = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit);
			AddRequiredCosignersKeys(sufficientCosigners, *pSubTransaction);
			AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosigners);
		}
	}

	TEST(TEST_CLASS, SingleAddModificationRequiresMinApproval) {
		AssertMinApprovalLimit(3, { Add }, 3, 4);
	}

	TEST(TEST_CLASS, MultipleAddModificationsRequireMinApproval) {
		AssertMinApprovalLimit(3, { Add, Add, Add }, 3, 4);
	}

	TEST(TEST_CLASS, SingleDelModificationRequiresMinRemoval) {
		AssertMinApprovalLimit(4, { Del }, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinRemovalWhenMinRemovalIsGreater) {
		AssertMinApprovalLimit(4, { Add, Del, Add }, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinApprovalWhenMinApprovalIsGreater) {
		AssertMinApprovalLimit(4, { Add, Del, Add }, 4, 3);
	}

	TEST(TEST_CLASS, SingleUnknownModificationRequiresMinApproval) {
		AssertMinApprovalLimit(3, { static_cast<model::CosignatoryModificationType>(0xCC) }, 3, 4);
	}

	TEST(TEST_CLASS, NonCosignatoryModificationRequiresMinApproval) {
		AssertMinApprovalLimit(3, {}, 3, 4);
	}

	TEST(TEST_CLASS, MinRemovalLimitIsAppliedAcrossMultipleLevels) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomByteArray<Key>();
		auto aggregateSigner = test::GenerateRandomByteArray<Key>();
		auto cosignatories = test::GenerateRandomDataVector<Key>(3);
		auto secondLevelCosignatories = test::GenerateRandomDataVector<Key>(4);
		auto base23Cosigners = std::vector<Key>{
			cosignatories[0], cosignatories[1],
			secondLevelCosignatories[0], secondLevelCosignatories[1], secondLevelCosignatories[2]
		};

		auto pSubTransaction = test::CreateModifyMultisigAccountTransaction(embeddedSigner, { Del });

		// - create the cache making the embedded signer a 2-3-3 multisig where the second cosignatory is a 3-4-4 multisig
		auto cache = CreateCacheWithMultilevelMultisig(embeddedSigner, cosignatories, secondLevelCosignatories);

		// Assert: 3 first-level and 4 second-level cosigners are required for approval
		CATAPULT_LOG(debug) << "running test with 3 + 3 cosigners (insufficient)";
		auto insufficient33Cosigners = base23Cosigners;
		insufficient33Cosigners.push_back(cosignatories[2]);
		AssertValidationResult(Failure_Aggregate_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficient33Cosigners);

		CATAPULT_LOG(debug) << "running test with 2 + 4 cosigners (insufficient)";
		auto insufficient24Cosigners = base23Cosigners;
		insufficient24Cosigners.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(Failure_Aggregate_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficient24Cosigners);

		CATAPULT_LOG(debug) << "running test with 3 + 4 cosigners (sufficient)";
		auto sufficientCosigners = base23Cosigners;
		sufficientCosigners.push_back(cosignatories[2]);
		sufficientCosigners.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosigners);
	}

	// endregion

	// region cosignatory approval - utils

	namespace {
		enum class AccountPolicy { Regular, Multisig };

		void AddSingleLevelMultisig(cache::CatapultCache& cache, const Key& multisigPublicKey, const std::vector<Key>& cosignatories) {
			auto cacheDelta = cache.createDelta();
			test::MakeMultisig(cacheDelta, multisigPublicKey, cosignatories, 3, 3); // make a (3-3-X default) multisig
			cache.commit(Height());
		}

		void AddAll(std::vector<Key>& allCosigners, const std::vector<Key>& cosigners) {
			allCosigners.insert(allCosigners.cend(), cosigners.cbegin(), cosigners.cend());
		}

		template<typename TMergeKeys>
		void AssertCosignatoriesMustApproveTransaction(
				ValidationResult expectedResult,
				const std::vector<model::CosignatoryModificationType>& modificationTypes,
				const std::vector<AccountPolicy>& accountPolicies,
				TMergeKeys mergeKeys) {
			// Sanity:
			ASSERT_EQ(modificationTypes.size(), accountPolicies.size());

			// Arrange:
			auto embeddedSigner = test::GenerateRandomByteArray<Key>();
			auto aggregateSigner = test::GenerateRandomByteArray<Key>();
			auto embeddedSignerCosignatories = test::GenerateRandomDataVector<Key>(3);

			auto pSubTransaction = test::CreateModifyMultisigAccountTransaction(embeddedSigner, modificationTypes);

			// - create the cache making the embedded signer a single level multisig
			auto cache = test::MultisigCacheFactory::Create();
			AddSingleLevelMultisig(cache, embeddedSigner, embeddedSignerCosignatories);

			// - make added cosigners single level multisig according to the multisig policies
			std::vector<Key> requiredCosignatories;
			auto numModifications = pSubTransaction->ModificationsCount;
			auto* pModification = pSubTransaction->ModificationsPtr();
			for (auto i = 0u; i < numModifications; ++i, ++pModification) {
				auto isAdded = model::CosignatoryModificationType::Add == pModification->ModificationType;
				if (isAdded && AccountPolicy::Multisig == accountPolicies[i]) {
					// - CosignatoryPublicKey is not a required cosigner because it is a multisig account
					auto cosignerCosignatories = test::GenerateRandomDataVector<Key>(3);
					AddSingleLevelMultisig(cache, pModification->CosignatoryPublicKey, cosignerCosignatories);
					AddAll(requiredCosignatories, cosignerCosignatories);
				} else {
					requiredCosignatories.push_back(pModification->CosignatoryPublicKey);
				}
			}

			// Assert:
			auto cosigners = mergeKeys(embeddedSignerCosignatories, requiredCosignatories);
			AssertValidationResult(expectedResult, cache, aggregateSigner, *pSubTransaction, cosigners);
		}

		template<typename TTraits>
		void AssertCosignatoriesMustApproveTransaction(const std::vector<AccountPolicy>& accountPolicies) {
			// Assert:
			AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, { Add, Add, Add }, accountPolicies, TTraits::MergeKeys);
		}

		struct ValidationFailureTraits {
			static constexpr auto ExpectedResult = Failure_Aggregate_Missing_Cosigners;

			static auto MergeKeys(const std::vector<Key>& embeddedSignerCosignatories, const std::vector<Key>&) {
				return embeddedSignerCosignatories;
			}
		};

		struct ValidationSuccessTraits {
			static constexpr auto ExpectedResult = ValidationResult::Success;

			static auto MergeKeys(const std::vector<Key>& embeddedSignerCosignatories, const std::vector<Key>& requiredCosignatories) {
				auto cosigners = std::vector<Key>(embeddedSignerCosignatories.cbegin(), embeddedSignerCosignatories.cend());
				AddAll(cosigners, requiredCosignatories);
				return cosigners;
			}
		};
	}

	// endregion

	// region cosignatory approval - shared

#define COSIGNER_APPROVAL_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Failure) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationFailureTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Success) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationSuccessTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_SingleAdd_NotMultisig) {
		AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, { Add }, { AccountPolicy::Regular }, TTraits::MergeKeys);
	}

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_SingleAdd_Multisig) {
		AssertCosignatoriesMustApproveTransaction(TTraits::ExpectedResult, { Add }, { AccountPolicy::Multisig }, TTraits::MergeKeys);
	}

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_NoneMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Regular, AccountPolicy::Regular, AccountPolicy::Regular });
	}

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_SomeMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Multisig, AccountPolicy::Regular, AccountPolicy::Multisig });
	}

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_MultipleAdds_AllMultisig) {
		AssertCosignatoriesMustApproveTransaction<TTraits>({ AccountPolicy::Multisig, AccountPolicy::Multisig, AccountPolicy::Multisig });
	}

	COSIGNER_APPROVAL_TEST(CosignatoryApproval_AddsAndDelete_SomeMultisig) {
		AssertCosignatoriesMustApproveTransaction(
				TTraits::ExpectedResult,
				{ Add, Del, Add },
				{ AccountPolicy::Multisig, AccountPolicy::Multisig, AccountPolicy::Regular },
				TTraits::MergeKeys);
	}

	// endregion

	// region cosignatory approval - failure

	TEST(TEST_CLASS, InsuffientWhenOnlySomeCosignatoriesApprove) {
		AssertCosignatoriesMustApproveTransaction(
				Failure_Aggregate_Missing_Cosigners,
				{ Add, Del, Add },
				{ AccountPolicy::Multisig, AccountPolicy::Multisig, AccountPolicy::Regular },
				[](const auto& embeddedSignerCosignatories, const auto& requiredCosignatories) {
					auto cosigners = ValidationSuccessTraits::MergeKeys(embeddedSignerCosignatories, requiredCosignatories);
					cosigners.erase(--cosigners.cend());
					return cosigners;
				});
	}

	// endregion

	// region cosignatory approval - success

	TEST(TEST_CLASS, SufficientWhenOnlyDeleteModification_NotMultisig) {
		constexpr auto Success = ValidationResult::Success;
		AssertCosignatoriesMustApproveTransaction(Success, { Del }, { AccountPolicy::Regular }, [](const auto& cosigners, const auto&) {
			return cosigners;
		});
	}

	TEST(TEST_CLASS, SufficientWhenOnlyDeleteModification_Multisig) {
		constexpr auto Success = ValidationResult::Success;
		AssertCosignatoriesMustApproveTransaction(Success, { Del }, { AccountPolicy::Multisig }, [](const auto& cosigners, const auto&) {
			return cosigners;
		});
	}

	// endregion
}}
