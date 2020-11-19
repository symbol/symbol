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

#include "catapult/ionet/RateMonitor.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS RateMonitorTests

	// region TestContext

	namespace {
		class TestContext {
		public:
			explicit TestContext(const std::vector<uint32_t>& rawTimestamps)
					: m_numRateExceededTriggers(0)
					, m_monitor(
							{ 5, utils::TimeSpan::FromMilliseconds(111), utils::FileSize::FromBytes(5 * 1234) },
							test::CreateTimeSupplierFromMilliseconds(rawTimestamps),
							[&numRateExceededTriggers = m_numRateExceededTriggers]() { ++numRateExceededTriggers; })
			{}

		public:
			size_t numRateExceededTriggers() const {
				return m_numRateExceededTriggers;
			}

			const auto& monitor() const {
				return m_monitor;
			}

		public:
			void acceptAll(std::initializer_list<uint32_t> sizes) {
				for (auto size : sizes)
					m_monitor.accept(size);
			}

		private:
			size_t m_numRateExceededTriggers;
			RateMonitor m_monitor;
		};
	}

	// endregion

	// region zero buckets

	TEST(TEST_CLASS, RateMonitorIsInitiallyEmpty) {
		// Arrange:
		TestContext context({ 1 });

		// Assert:
		EXPECT_EQ(utils::FileSize(), context.monitor().totalSize());
		EXPECT_EQ(0u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	// endregion

	// region single bucket

	TEST(TEST_CLASS, CanAddSingleSizeToSingleBucket) {
		// Arrange:
		TestContext context({ 1 });

		// Act:
		context.acceptAll({ 123 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(123), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanAddMultipleSizesToSingleBucket) {
		// Arrange: add sizes with timestamps ranging from beginning to end of single bucket
		TestContext context({ 1, 1, 50, 111, 111 });

		// Act:
		context.acceptAll({ 7, 9, 1, 3, 5 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(25), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanAddMultipleSizesToSingleBucket_RelativeToStartTime) {
		// Arrange:
		TestContext context({ 50, 112, 151 });

		// Act:
		context.acceptAll({ 10, 20, 40 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(70), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, RateIsNotTriggeredBySingleBucketWhenMax) {
		// Arrange:
		TestContext context({ 1, 1 });

		// Act:
		context.acceptAll({ 5 * 1000, 5 * 234 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(5 * 1234), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, RateIsTriggeredBySingleBucketWhenMaxIsExceeded) {
		// Arrange:
		TestContext context({ 1, 1 });

		// Act:
		context.acceptAll({ 5 * 1000, 5 * 234 + 1 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(5 * 1234 + 1), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(1u, context.numRateExceededTriggers());
	}

	// endregion

	// region multiple buckets

	TEST(TEST_CLASS, CanAddSingleSizeToMultipleBuckets) {
		// Arrange:
		TestContext context({ 1, 112, 223 });

		// Act:
		context.acceptAll({ 123, 50, 10 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(183), context.monitor().totalSize());
		EXPECT_EQ(3u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanAddSingleSizeToMultipleBuckets_RelativeToStartTime) {
		// Arrange:
		TestContext context({ 50, 223, 262 });

		// Act:
		context.acceptAll({ 123, 50, 10 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(183), context.monitor().totalSize());
		EXPECT_EQ(2u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanAddSingleSizeToMultipleBucketsWithGaps) {
		// Arrange:
		TestContext context({ 1, 223, 556 });

		// Act:
		context.acceptAll({ 123, 50, 10 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(183), context.monitor().totalSize());
		EXPECT_EQ(3u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanAddSingleSizeToMaxBuckets) {
		// Arrange:
		TestContext context({ 1, 112, 223, 334, 445, 556 });

		// Act:
		context.acceptAll({ 10, 20, 40, 30, 50, 60 });

		// Assert: there can be at most NumBuckets + 1 buckets
		EXPECT_EQ(utils::FileSize::FromBytes(210), context.monitor().totalSize());
		EXPECT_EQ(6u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, RateIsNotTriggeredByMultipleBucketsWhenMax) {
		// Arrange:
		TestContext context({ 1, 556 });

		// Act:
		context.acceptAll({ 5 * 1000, 5 * 234 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(5 * 1234), context.monitor().totalSize());
		EXPECT_EQ(2u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, RateIsTriggeredByMultipleBucketsWhenMaxIsExceeded) {
		// Arrange:
		TestContext context({ 1, 556 });

		// Act:
		context.acceptAll({ 5 * 1000, 5 * 234 + 1 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(5 * 1234 + 1), context.monitor().totalSize());
		EXPECT_EQ(2u, context.monitor().bucketsSize());
		EXPECT_EQ(1u, context.numRateExceededTriggers());
	}

	// endregion

	// region multiple buckets - pruning

	TEST(TEST_CLASS, CanPruneSingleBucket) {
		// Arrange:
		TestContext context({ 1, 223, 557 });

		// Act:
		context.acceptAll({ 123, 50, 10 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(60), context.monitor().totalSize());
		EXPECT_EQ(2u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, CanPruneMultipleBuckets) {
		// Arrange:
		TestContext context({ 1, 223, 1000 });

		// Act:
		context.acceptAll({ 123, 50, 10 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(10), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	TEST(TEST_CLASS, RateTriggeringRespectsPruning) {
		// Arrange:
		TestContext context({ 1, 557 });

		// Act:
		context.acceptAll({ 5 * 1000, 5 * 234 + 1 });

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(5 * 234 + 1), context.monitor().totalSize());
		EXPECT_EQ(1u, context.monitor().bucketsSize());
		EXPECT_EQ(0u, context.numRateExceededTriggers());
	}

	// endregion
}}
