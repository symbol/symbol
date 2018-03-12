#include "src/validators/Validators.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateHashPresentValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AggregateHashPresent,)

	namespace {
		auto CreateNotification(const Hash256& transactionHash, model::EntityType transactionType) {
			return model::TransactionNotification(Key(), transactionHash, transactionType, Timestamp());
		}

		auto CreateCache(const Hash256& transactionHash, Height expirationHeight, model::LockStatus status = model::LockStatus::Unused) {
			auto cache = test::HashLockInfoCacheFactory::Create();
			auto cacheDelta = cache.createDelta();
			auto& lockInfoCacheDelta = cacheDelta.sub<cache::HashLockInfoCache>();
			auto lockInfo = test::BasicHashLockInfoTestTraits::CreateLockInfo(expirationHeight);
			lockInfo.Hash = transactionHash;
			lockInfo.Status = status;
			lockInfoCacheDelta.insert(lockInfo);
			cache.commit(Height());
			return cache;
		}

		auto RunValidator(
				cache::CatapultCache& cache,
				const Hash256& transactionHash,
				model::EntityType transactionType,
				Height notificationHeight) {
			// Arrange:
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(notificationHeight, readOnlyCache);
			auto pValidator = CreateAggregateHashPresentValidator();

			// Act:
			return test::ValidateNotification<model::TransactionNotification>(
					*pValidator,
					CreateNotification(transactionHash, transactionType),
					context);
		}
	}

	TEST(TEST_CLASS, SuccessForNonAggregateBondedTransactionType) {
		// Arrange:
		auto cache = CreateCache(test::GenerateRandomData<Hash256_Size>(), Height());

		// Act:
		auto result = RunValidator(cache, test::GenerateRandomData<Hash256_Size>(), model::Entity_Type_Aggregate_Complete, Height());

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, SuccessIfLockInfoMeetsAllConditions) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto cache = CreateCache(hash, Height(123));

		// Act:
		auto result = RunValidator(cache, hash, model::Entity_Type_Aggregate_Bonded, Height(120));

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, FailureForUnknownTransactionHash) {
		// Arrange:
		auto cache = CreateCache(test::GenerateRandomData<Hash256_Size>(), Height());

		// Act:
		auto result = RunValidator(cache, test::GenerateRandomData<Hash256_Size>(), model::Entity_Type_Aggregate_Bonded, Height());

		// Assert:
		EXPECT_EQ(Failure_Lock_Hash_Does_Not_Exist, result);
	}

	TEST(TEST_CLASS, FailureIfLockInfoIsExpired_AtExpirationHeight) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto cache = CreateCache(hash, Height(123));

		// Act:
		auto result = RunValidator(cache, hash, model::Entity_Type_Aggregate_Bonded, Height(123));

		// Assert:
		EXPECT_EQ(Failure_Lock_Inactive_Hash, result);
	}

	TEST(TEST_CLASS, FailureIfLockInfoIsExpired_AfterExpirationHeight) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto cache = CreateCache(hash, Height(123));

		// Act:
		auto result = RunValidator(cache, hash, model::Entity_Type_Aggregate_Bonded, Height(150));

		// Assert:
		EXPECT_EQ(Failure_Lock_Inactive_Hash, result);
	}

	TEST(TEST_CLASS, FailureIfTransactionHashIsAlreadyUsed) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto cache = CreateCache(hash, Height(123), model::LockStatus::Used);

		// Act:
		auto result = RunValidator(cache, hash, model::Entity_Type_Aggregate_Bonded, Height());

		// Assert:
		EXPECT_EQ(Failure_Lock_Hash_Already_Used, result);
	}
}}
