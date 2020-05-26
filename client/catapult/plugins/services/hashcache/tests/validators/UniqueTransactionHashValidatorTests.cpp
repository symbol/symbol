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

#define TEST_CLASS UniqueTransactionHashValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(UniqueTransactionHash,)

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;

		void AssertValidationResult(
				ValidationResult expectedResult,
				const CatapultCache& cache,
				const state::TimestampedHash& timestampedHash) {
			// Arrange: need to copy timestampedHash.Hash because it is an std::array, not a Hash256
			auto pValidator = CreateUniqueTransactionHashValidator();
			auto transactionHash = Hash256(timestampedHash.Hash);
			auto notification = model::TransactionNotification(Address(), transactionHash, model::EntityType(), timestampedHash.Time);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		using TimestampedHashes = std::vector<state::TimestampedHash>;

		TimestampedHashes CreateTimestampedHashes(size_t count) {
			TimestampedHashes timestampedHashes;
			timestampedHashes.reserve(count);
			for (auto i = 0u; i < count; ++i)
				timestampedHashes.push_back(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));

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

	TEST(TEST_CLASS, SuccessWhenValidatingEntityWithNeitherMatchingTimestampNorHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(100), test::GenerateRandomByteArray<Hash256>());

		// Assert:
		AssertValidationResult(Success_Result, cache, notificationTimestampedHash);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityWithMatchingTimestampButNotHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(5), test::GenerateRandomByteArray<Hash256>());

		// Assert:
		AssertValidationResult(Success_Result, cache, notificationTimestampedHash);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityWithMatchingHashButNotTimestamp) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);
		auto notificationTimestampedHash = state::TimestampedHash(Timestamp(25), timestampedHashes[5].Hash);

		// Assert:
		AssertValidationResult(Success_Result, cache, notificationTimestampedHash);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEntityWithMatchingTimestampAndHash) {
		// Arrange:
		auto timestampedHashes = CreateTimestampedHashes(10);
		auto cache = CreateCache(timestampedHashes);

		// Assert:
		for (const auto& notificationTimestampedHash : timestampedHashes)
			AssertValidationResult(Failure_Hash_Already_Exists, cache, notificationTimestampedHash);
	}
}}
