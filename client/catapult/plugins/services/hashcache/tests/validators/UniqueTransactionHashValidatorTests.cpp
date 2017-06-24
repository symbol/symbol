#include "src/validators/Validators.h"
#include "src/cache/HashCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/HashCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;
using namespace catapult::cache;

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(UniqueTransactionHash,)

	namespace {
		void AssertValidationResult(
				const CatapultCache& cache,
				const state::TimestampedHash& timestampedHash,
				ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateUniqueTransactionHashValidator();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(1), readOnlyCache);
			auto notification = model::TransactionNotification(Key(), timestampedHash.Hash, timestampedHash.Time);

			// Act:
			auto result = pValidator->validate(notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		using TimestampedHashes = std::vector<state::TimestampedHash>;

		TimestampedHashes CreateTimestampedHashes(size_t count) {
			TimestampedHashes timestampedHashes;
			timestampedHashes.reserve(count);
			for (auto i = 0u; i < count; ++i)
				timestampedHashes.push_back(state::TimestampedHash(Timestamp(i), test::GenerateRandomData<Hash256_Size>()));

			return timestampedHashes;
		}

		CatapultCache CreateCache(const TimestampedHashes& timestampedHashes) {
			auto cache = test::CreateEmptyCatapultCache<test::HashCacheFactory>(model::BlockChainConfiguration::Uninitialized());
			auto delta = cache.createDelta();
			auto& hashCache = delta.sub<cache::HashCache>();
			for (const auto& timestampedHash : timestampedHashes)
				hashCache.insert(timestampedHash);

			cache.commit(Height());
			return cache;
		}
	}

	TEST(UniqueTransactionHashValidatorTests, SuccessWhenValidatingEntityWithNeitherMatchingTimestampNorHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(100), test::GenerateRandomData<Hash256_Size>());

		// Assert:
		AssertValidationResult(cache, notificationTimestampedHash, ValidationResult::Success);
	}

	TEST(UniqueTransactionHashValidatorTests, SuccessWhenValidatingEntityWithMatchingTimestampButNotHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(5), test::GenerateRandomData<Hash256_Size>());

		// Assert:
		AssertValidationResult(cache, notificationTimestampedHash, ValidationResult::Success);
	}

	TEST(UniqueTransactionHashValidatorTests, SuccessWhenValidatingEntityWithMatchingHashButNotTimestamp) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(25), timestampedHashes[5].Hash);

		// Assert:
		AssertValidationResult(cache, notificationTimestampedHash, ValidationResult::Success);
	}

	TEST(UniqueTransactionHashValidatorTests, FailureWhenValidatingEntityWithMatchingTimestampAndHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);

		// Assert:
		for (const auto& notificationTimestampedHash : timestampedHashes)
			AssertValidationResult(cache, notificationTimestampedHash, Failure_Hash_Exists);
	}
}}
