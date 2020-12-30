/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/state/AccountActivityBuckets.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountActivityBucketsTests

	namespace {
		using ActivityBucketTuple = std::tuple<uint64_t, uint64_t, uint32_t, uint64_t>;

		// region test utils

		void AssertEqual(
				const ActivityBucketTuple& expectedTuple,
				const AccountActivityBuckets::ActivityBucket& actualBucket,
				const std::string message) {
			// Assert:
			EXPECT_EQ(model::ImportanceHeight(std::get<0>(expectedTuple)), actualBucket.StartHeight) << message;
			EXPECT_EQ(Amount(std::get<1>(expectedTuple)), actualBucket.TotalFeesPaid) << message;
			EXPECT_EQ(std::get<2>(expectedTuple), actualBucket.BeneficiaryCount) << message;
			EXPECT_EQ(std::get<3>(expectedTuple), actualBucket.RawScore) << message;
		}

		void AssertHistoricalValues(
				const AccountActivityBuckets& buckets,
				const std::array<ActivityBucketTuple, Activity_Bucket_History_Size>& expectedActivityBucketTuples) {
			auto index = 0u;
			for (const auto& bucket : buckets) {
				const auto message = "height " + std::to_string(bucket.StartHeight.unwrap()) + ", index " + std::to_string(index);
				const auto& expectedTuple = expectedActivityBucketTuples[index];

				// Assert: iteration
				AssertEqual(expectedTuple, bucket, message + " (iteration)");

				// - lookup availability
				AssertEqual(expectedTuple, buckets.get(bucket.StartHeight), message + " (lookup)");
				++index;
			}

			// - expected number of buckets
			EXPECT_EQ(Activity_Bucket_History_Size, index);
		}

		void AssertEmpty(const AccountActivityBuckets& buckets) {
			EXPECT_TRUE(buckets.empty());
			AssertHistoricalValues(buckets, {{
				std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
				std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
				std::make_tuple(0, 0, 0, 0)
			}});
		}

		// endregion

		// region Set / TrySet

		void Set(
				AccountActivityBuckets& buckets,
				model::ImportanceHeight startHeight,
				Amount totalFeesPaid,
				uint32_t beneficiaryCount,
				uint64_t rawScore) {
			buckets.update(startHeight, [totalFeesPaid, beneficiaryCount, rawScore](auto& bucket) {
				bucket.TotalFeesPaid = totalFeesPaid;
				bucket.BeneficiaryCount = beneficiaryCount;
				bucket.RawScore = rawScore;
			});
		}

		bool TrySet(
				AccountActivityBuckets& buckets,
				model::ImportanceHeight startHeight,
				Amount totalFeesPaid,
				uint32_t beneficiaryCount,
				uint64_t rawScore) {
			return buckets.tryUpdate(startHeight, [totalFeesPaid, beneficiaryCount, rawScore](auto& bucket) {
				bucket.TotalFeesPaid = totalFeesPaid;
				bucket.BeneficiaryCount = beneficiaryCount;
				bucket.RawScore = rawScore;
			});
		}

		// endregion
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateBuckets) {
		// Act:
		AccountActivityBuckets buckets;

		// Assert:
		AssertEmpty(buckets);
	}

	// endregion

	// region update

	TEST(TEST_CLASS, CannotUpdateBucketAtLowerHeight) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act + Assert:
		EXPECT_THROW(Set(buckets, model::ImportanceHeight(233), Amount(20), 11, 7), catapult_runtime_error);
		EXPECT_THROW(Set(buckets, model::ImportanceHeight(100), Amount(20), 11, 7), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanUpdateBucketAtSameHeight) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act:
		Set(buckets, model::ImportanceHeight(234), Amount(18), 11, 14);

		// Assert:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(234, 18, 11, 14), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CanUpdateBucketAtHigherHeight) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act:
		Set(buckets, model::ImportanceHeight(235), Amount(18), 11, 14);

		// Assert:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(235, 18, 11, 14), std::make_tuple(234, 20, 10, 15), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CanSetMaxHistoricalBuckets) {
		// Act:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(111), Amount(21), 99, 15);
		Set(buckets, model::ImportanceHeight(222), Amount(22), 88, 20);
		Set(buckets, model::ImportanceHeight(333), Amount(23), 77, 35);
		Set(buckets, model::ImportanceHeight(444), Amount(24), 66, 40);
		Set(buckets, model::ImportanceHeight(555), Amount(25), 55, 55);
		Set(buckets, model::ImportanceHeight(666), Amount(29), 44, 60);
		Set(buckets, model::ImportanceHeight(777), Amount(28), 33, 75);
		Set(buckets, model::ImportanceHeight(888), Amount(27), 22, 80);
		Set(buckets, model::ImportanceHeight(999), Amount(26), 11, 95);

		// Assert: roll over after max (8) is set
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(999, 26, 11, 95), std::make_tuple(888, 27, 22, 80), std::make_tuple(777, 28, 33, 75),
			std::make_tuple(666, 29, 44, 60), std::make_tuple(555, 25, 55, 55), std::make_tuple(444, 24, 66, 40),
			std::make_tuple(333, 23, 77, 35)
		}});
	}

	TEST(TEST_CLASS, CanUpdateAfterPush) {
		// Arrange:
		AccountActivityBuckets buckets;

		// Act:
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);
		buckets.push();
		Set(buckets, model::ImportanceHeight(235), Amount(18), 11, 14);

		// Assert:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(235, 18, 11, 14), std::make_tuple(0, 0, 0, 0), std::make_tuple(234, 20, 10, 15),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	// endregion

	// region tryUpdate

	TEST(TEST_CLASS, CannotUpdateBucketAtLowerHeight_TryUpdate) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act + Assert:
		EXPECT_THROW(TrySet(buckets, model::ImportanceHeight(233), Amount(20), 11, 7), catapult_runtime_error);
		EXPECT_THROW(TrySet(buckets, model::ImportanceHeight(100), Amount(20), 11, 7), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanUpdateBucketAtSameHeight_TryUpdate) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act:
		auto tryResult = TrySet(buckets, model::ImportanceHeight(234), Amount(18), 11, 14);

		// Assert:
		EXPECT_TRUE(tryResult);
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(234, 18, 11, 14), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CannotUpdateBucketAtHigherHeight_TryUpdate) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act:
		auto tryResult = TrySet(buckets, model::ImportanceHeight(235), Amount(18), 11, 14);

		// Assert:
		EXPECT_FALSE(tryResult);
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(234, 20, 10, 15), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CannotUpdateAfterPush_TryUpdate) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Act:
		buckets.push();
		auto tryResult = TrySet(buckets, model::ImportanceHeight(235), Amount(18), 11, 14);

		// Assert:
		EXPECT_FALSE(tryResult);
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(0, 0, 0, 0), std::make_tuple(234, 20, 10, 15), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	// endregion

	// region push / pop

	namespace {
		void Fill(AccountActivityBuckets& buckets) {
			// Arrange:
			Set(buckets, model::ImportanceHeight(777), Amount(28), 33, 10);
			Set(buckets, model::ImportanceHeight(888), Amount(27), 22, 15);
			Set(buckets, model::ImportanceHeight(999), Amount(26), 11, 20);

			// Sanity:
			EXPECT_EQ(model::ImportanceHeight(999), buckets.get(model::ImportanceHeight(999)).StartHeight);
		}
	}

	TEST(TEST_CLASS, CanPushBucketWhenEmpty) {
		// Act:
		AccountActivityBuckets buckets;
		buckets.push();

		// Assert:
		AssertEmpty(buckets);
	}

	TEST(TEST_CLASS, CanPushBucketWhenNotEmpty) {
		// Arrange:
		AccountActivityBuckets buckets;
		Fill(buckets);

		// Act:
		buckets.push();

		// Assert:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(0, 0, 0, 0), std::make_tuple(999, 26, 11, 20), std::make_tuple(888, 27, 22, 15),
			std::make_tuple(777, 28, 33, 10), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CanPushAllBuckets) {
		// Arrange:
		AccountActivityBuckets buckets;
		Fill(buckets);

		// Act:
		for (auto i = 0u; i < Activity_Bucket_History_Size; ++i)
			buckets.push();

		// Assert:
		AssertEmpty(buckets);
	}

	TEST(TEST_CLASS, CanPopMostRecentBucket) {
		// Arrange:
		AccountActivityBuckets buckets;
		Fill(buckets);

		// Act:
		buckets.pop();

		// Assert:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(888, 27, 22, 15), std::make_tuple(777, 28, 33, 10), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});
	}

	TEST(TEST_CLASS, CanPopAllBuckets) {
		// Arrange:
		AccountActivityBuckets buckets;
		Fill(buckets);

		// Act:
		for (auto i = 0u; i < 3u; ++i)
			buckets.pop();

		// Assert:
		AssertEmpty(buckets);
	}

	// endregion

	// region get

	TEST(TEST_CLASS, CannotRetrieveUnknownHistoricalBucket) {
		// Arrange:
		AccountActivityBuckets buckets;
		Set(buckets, model::ImportanceHeight(234), Amount(20), 10, 15);

		// Sanity:
		EXPECT_FALSE(buckets.empty());
		AssertHistoricalValues(buckets, {{
			std::make_tuple(234, 20, 10, 15), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 0, 0, 0),
			std::make_tuple(0, 0, 0, 0)
		}});

		// Act + Assert:
		EXPECT_EQ(model::ImportanceHeight(), buckets.get(model::ImportanceHeight()).StartHeight);
		EXPECT_EQ(model::ImportanceHeight(), buckets.get(model::ImportanceHeight(233)).StartHeight);
		EXPECT_EQ(model::ImportanceHeight(), buckets.get(model::ImportanceHeight(235)).StartHeight);
	}

	// endregion
}}

