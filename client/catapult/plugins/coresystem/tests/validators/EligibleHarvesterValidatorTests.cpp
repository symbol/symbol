#include "src/validators/Validators.h"
#include "src/cache/AccountStateCache.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(EligibleHarvester, Amount(1234))

	namespace {
		constexpr auto Importance_Grouping = 234u;

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Importance_Grouping);
		}

		auto CreateEmptyCatapultCache() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = Importance_Grouping;
			return test::CreateEmptyCatapultCache(config);
		}

		void AddAccount(
				cache::CatapultCache& cache,
				const Key& publicKey,
				Importance importance,
				model::ImportanceHeight importanceHeight,
				Amount balance) {
			auto delta = cache.createDelta();
			auto pState = delta.sub<cache::AccountStateCache>().addAccount(publicKey, Height(100));
			pState->ImportanceInfo.set(importance, importanceHeight);
			pState->Balances.credit(Xem_Id, balance);
			cache.commit(Height());
		}
	}

	TEST(EligibleHarvesterValidatorTests, FailureIfAccountIsUnknown) {
		// Arrange:
		auto cache = CreateEmptyCatapultCache();
		auto key = test::GenerateRandomData<Key_Size>();
		auto height = Height(1000);
		AddAccount(cache, key, Importance(1000), ConvertToImportanceHeight(height), Amount(9999));

		auto cacheView = cache.createView();
		auto readOnlyCache = cacheView.toReadOnly();
		auto pValidator = CreateEligibleHarvesterValidator(Amount(1234));
		auto context = test::CreateValidatorContext(height, readOnlyCache);

		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = test::CreateBlockNotification(signer);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification, context);

		// Assert:
		EXPECT_EQ(Failure_Core_Block_Harvester_Ineligible, result);
	}

	namespace {
		void AssertValidationResult(
				int64_t minBalanceDelta,
				Importance importance,
				model::ImportanceHeight importanceHeight,
				Height blockHeight,
				ValidationResult expectedResult) {
			// Arrange:
			auto cache = CreateEmptyCatapultCache();
			auto key = test::GenerateRandomData<Key_Size>();
			auto initialBalance = Amount(static_cast<Amount::ValueType>(1234 + minBalanceDelta));
			AddAccount(cache, key, importance, importanceHeight, initialBalance);

			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto pValidator = CreateEligibleHarvesterValidator(Amount(1234));
			auto context = test::CreateValidatorContext(blockHeight, readOnlyCache);

			auto notification = test::CreateBlockNotification(key);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(EligibleHarvesterValidatorTests, FailureIfBalanceIsBelowMinBalance) {
		// Assert:
		auto height = Height(10000);
		auto expectedResult = Failure_Core_Block_Harvester_Ineligible;
		AssertValidationResult(-1, Importance(123), ConvertToImportanceHeight(height), height, expectedResult);
		AssertValidationResult(-100, Importance(123), ConvertToImportanceHeight(height), height, expectedResult);
	}

	TEST(EligibleHarvesterValidatorTests, FailureIfImportanceIsZero) {
		// Assert:
		auto height = Height(10000);
		auto expectedResult = Failure_Core_Block_Harvester_Ineligible;
		AssertValidationResult(12345, Importance(0), ConvertToImportanceHeight(height), height, expectedResult);
	}

	TEST(EligibleHarvesterValidatorTests, FailureIfImportanceIsNotSetAtCorrectHeight) {
		// Assert:
		auto expectedResult = Failure_Core_Block_Harvester_Ineligible;
		AssertValidationResult(12345, Importance(0), model::ImportanceHeight(123), Height(1234), expectedResult);
	}

	TEST(EligibleHarvesterValidatorTests, SuccessIfAllCriteriaAreMet) {
		// Assert:
		auto height = Height(10000);
		auto expectedResult = ValidationResult::Success;
		AssertValidationResult(0, Importance(123), ConvertToImportanceHeight(height), height, expectedResult);
		AssertValidationResult(1, Importance(123), ConvertToImportanceHeight(height), height, expectedResult);
		AssertValidationResult(12345, Importance(123), ConvertToImportanceHeight(height), height, expectedResult);
	}
}}
