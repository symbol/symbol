#include "src/validators/Validators.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigAggregateSufficientCosignersValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigAggregateSufficientCosigners,)

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const Key& signer,
				const model::EmbeddedEntity& subTransaction,
				const std::vector<Key>& cosigners) {
			// Arrange:
			// - setup cosignatures
			auto cosignatures = test::GenerateCosignaturesFromCosigners(cosigners);

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(), readOnlyCache);

			using Notification = model::AggregateEmbeddedTransactionNotification;
			Notification notification(signer, subTransaction, cosignatures.size(), cosignatures.data());
			auto pValidator = CreateMultisigAggregateSufficientCosignersValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		std::unique_ptr<model::EmbeddedEntity> CreateEmbeddedTransaction(const Key& signer) {
			auto pTransaction = std::make_unique<model::EmbeddedEntity>();
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
				test::MakeMultisig(cacheDelta, test::GenerateRandomData<Key_Size>(), { aggregateSigner });

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
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(2);

		// Assert: aggregate signer (implicit cosigner) is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, aggregateSigner, aggregateSigner, cosigners);
	}

	NON_MULTISIG_TRAITS_TEST(SufficientWhenEmbeddedTransactionSignerIsCosigner) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomData<Key_Size>();
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
		auto cosigners = std::vector<Key>{ test::GenerateRandomData<Key_Size>(), embeddedSigner, test::GenerateRandomData<Key_Size>() };

		// Assert: one cosigner is same as embedded transaction signer
		AssertValidationResult<TTraits>(ValidationResult::Success, embeddedSigner, aggregateSigner, cosigners);
	}

	NON_MULTISIG_TRAITS_TEST(InsufficientWhenTransactionSignerIsNeitherAggregateSignerNorCosigner) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomData<Key_Size>();
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(2);

		// Assert: embedded transaction signer is neither aggregate signer nor cosigner
		AssertValidationResult<TTraits>(Failure_Multisig_Missing_Cosigners, embeddedSigner, aggregateSigner, cosigners);
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
			auto embeddedSigner = test::GenerateRandomData<Key_Size>();
			auto aggregateSigner = test::GenerateRandomData<Key_Size>();
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
		AssertBasicMultisigResult(
				ValidationResult::Success,
				[](const auto& cosignatories) { return std::vector<Key>{ cosignatories[0], cosignatories[2], cosignatories[3] }; });
	}

	TEST(TEST_CLASS, SufficientWhenMultisigEmbeddedTransactionSignerHasGreaterThanMinApprovers) {
		// Assert: 4 > 3
		AssertBasicMultisigResult(
				ValidationResult::Success,
				[](const auto& cosignatories) { return cosignatories; });
	}

	TEST(TEST_CLASS, InsufficientWhenMultisigEmbeddedTransactionSignerHasLessThanMinApprovers) {
		// Assert: 2 < 3
		AssertBasicMultisigResult(
				Failure_Multisig_Missing_Cosigners,
				[](const auto& cosignatories) { return std::vector<Key>{ cosignatories[0], cosignatories[2] }; });
	}

	TEST(TEST_CLASS, SufficientWhenMultisigEmbeddedTransactionSignerHasMinApproversIncludingAggregateSigner) {
		// Arrange: include the aggregate signer as a cosignatory
		auto embeddedSigner = test::GenerateRandomData<Key_Size>();
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
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
			auto embeddedSigner = test::GenerateRandomData<Key_Size>();
			auto aggregateSigner = test::GenerateRandomData<Key_Size>();
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
		AssertBasicMultilevelMultisigResult(
				ValidationResult::Success,
				[](const auto& cosignatories) { return std::vector<Key>{ cosignatories[0], cosignatories[2], cosignatories[3] }; });
	}

	TEST(TEST_CLASS, SufficientWhenMultilevelMultisigEmbeddedTransactionSignerHasGreaterThanMinApprovers) {
		// Assert: 4 > 3
		AssertBasicMultilevelMultisigResult(
				ValidationResult::Success,
				[](const auto& cosignatories) { return cosignatories; });
	}

	TEST(TEST_CLASS, InsufficientWhenMultilevelMultisigEmbeddedTransactionSignerHasLessThanMinApprovers) {
		// Assert: 2 < 3
		AssertBasicMultilevelMultisigResult(
				Failure_Multisig_Missing_Cosigners,
				[](const auto& cosignatories) { return std::vector<Key>{ cosignatories[0], cosignatories[2] }; });
	}

	TEST(TEST_CLASS, SuccessWhenMultisigTransactionSignerHasMinApproversSharedAcrossLevels) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomData<Key_Size>();
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
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

		void AssertMinApprovalLimit(
				uint32_t expectedLimit,
				const std::vector<model::CosignatoryModificationType>& modificationTypes,
				uint8_t minApproval,
				uint8_t minRemoval) {
			// Arrange:
			auto embeddedSigner = test::GenerateRandomData<Key_Size>();
			auto aggregateSigner = test::GenerateRandomData<Key_Size>();
			auto cosignatories = test::GenerateRandomDataVector<Key>(expectedLimit);

			auto pSubTransaction = test::CreateModifyMultisigAccountTransaction(embeddedSigner, modificationTypes);

			// - create the cache making the embedded signer a single level multisig
			auto cache = CreateCacheWithSingleLevelMultisig(embeddedSigner, cosignatories, minApproval, minRemoval);

			// Assert:
			CATAPULT_LOG(debug) << "running test with " << expectedLimit - 1 << " cosigners (insufficient)";
			auto insufficientCosigners = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit - 1);
			AssertValidationResult(Failure_Multisig_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficientCosigners);

			CATAPULT_LOG(debug) << "running test with " << expectedLimit << " cosigners (sufficient)";
			auto sufficientCosigners = std::vector<Key>(cosignatories.cbegin(), cosignatories.cbegin() + expectedLimit);
			AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosigners);
		}
	}

	TEST(TEST_CLASS, SingleAddModificationRequiresMinApproval) {
		// Assert:
		AssertMinApprovalLimit(3, { Add }, 3, 4);
	}

	TEST(TEST_CLASS, MultipleAddModificationsRequireMinApproval) {
		// Assert:
		AssertMinApprovalLimit(3, { Add, Add, Add }, 3, 4);
	}

	TEST(TEST_CLASS, SingleDelModificationRequiresMinRemoval) {
		// Assert:
		AssertMinApprovalLimit(4, { Del }, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinRemovalWhenMinRemovalIsGreater) {
		// Assert:
		AssertMinApprovalLimit(4, { Add, Del, Add }, 3, 4);
	}

	TEST(TEST_CLASS, MixedDelAndAddModificationsRequireMinApprovalWhenMinApprovalIsGreater) {
		// Assert:
		AssertMinApprovalLimit(4, { Add, Del, Add }, 4, 3);
	}

	TEST(TEST_CLASS, SingleUnknownModificationRequiresMinApproval) {
		// Assert:
		AssertMinApprovalLimit(3, { static_cast<model::CosignatoryModificationType>(0xCC) }, 3, 4);
	}

	TEST(TEST_CLASS, NonCosignatoryModificationRequiresMinApproval) {
		// Assert:
		AssertMinApprovalLimit(3, {}, 3, 4);
	}

	TEST(TEST_CLASS, MinRemovalLimitIsAppliedAcrossMultipleLevels) {
		// Arrange:
		auto embeddedSigner = test::GenerateRandomData<Key_Size>();
		auto aggregateSigner = test::GenerateRandomData<Key_Size>();
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
		AssertValidationResult(Failure_Multisig_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficient33Cosigners);

		CATAPULT_LOG(debug) << "running test with 2 + 4 cosigners (insufficient)";
		auto insufficient24Cosigners = base23Cosigners;
		insufficient24Cosigners.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(Failure_Multisig_Missing_Cosigners, cache, aggregateSigner, *pSubTransaction, insufficient24Cosigners);

		CATAPULT_LOG(debug) << "running test with 3 + 4 cosigners (sufficient)";
		auto sufficientCosigners = base23Cosigners;
		sufficientCosigners.push_back(cosignatories[2]);
		sufficientCosigners.push_back(secondLevelCosignatories[3]);
		AssertValidationResult(ValidationResult::Success, cache, aggregateSigner, *pSubTransaction, sufficientCosigners);
	}

	// endregion
}}
