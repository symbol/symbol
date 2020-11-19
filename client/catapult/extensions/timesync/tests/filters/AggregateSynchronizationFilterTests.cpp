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

#include "timesync/src/filters/SynchronizationFilters.h"
#include "timesync/src/filters/AggregateSynchronizationFilter.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync { namespace filters {

#define TEST_CLASS AggregateSynchronizationFilterTests

	// region normal filters

	namespace {
		SynchronizationFilter CreateCapturingSynchronizationFilter(
				std::vector<TimeSynchronizationSample>& capturedSamples,
				std::vector<int64_t>& capturedRawNodeAges,
				uint8_t id) {
			return [&capturedSamples, &capturedRawNodeAges, id](const auto& sample, auto nodeAge) {
				capturedSamples.push_back(sample);
				capturedRawNodeAges.push_back((static_cast<int8_t>(id) << 24) + nodeAge.unwrap());
				return false;
			};
		}

		void RunDelegationTest(size_t numFilters) {
			// Arrange:
			std::vector<SynchronizationFilter> filters;
			std::vector<TimeSynchronizationSample> capturedSamples;
			std::vector<int64_t> capturedRawNodeAges;
			for (uint8_t i = 0; i < numFilters; ++i)
				filters.push_back(CreateCapturingSynchronizationFilter(capturedSamples, capturedRawNodeAges, i));

			AggregateSynchronizationFilter aggregateFilter(filters);
			TimeSynchronizationSamples samples{ test::CreateTimeSyncSampleWithTimeOffset(5) };

			// Act:
			aggregateFilter(samples, NodeAge(123));

			// Assert:
			ASSERT_EQ(numFilters, aggregateFilter.size());
			for (auto i = 0u; i < numFilters; ++i) {
				auto message = "at index " + std::to_string(i);
				EXPECT_EQ(*samples.cbegin(), capturedSamples[i]) << message;
				EXPECT_EQ(123u, capturedRawNodeAges[i] & 0x00FFFFFF) << message;
				EXPECT_EQ(i, capturedRawNodeAges[i] >> 24) << message;
			}
		}
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundZeroFilters) {
		RunDelegationTest(0);
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundSingleFilter) {
		RunDelegationTest(1);
	}

	TEST(TEST_CLASS, CanCreateAggregateAroundMultipleFilters) {
		RunDelegationTest(3);
	}

	TEST(TEST_CLASS, AggregateFilterIsShortCircuiting) {
		// Arrange:
		std::vector<SynchronizationFilter> filters;
		std::vector<bool> capturedReturnValues;
		std::vector<bool> returnValues{ false, true, false, true };
		for (auto returnValue : returnValues) {
			filters.push_back([&capturedReturnValues, returnValue](const auto&, auto) {
				capturedReturnValues.push_back(returnValue);
				return returnValue;
			});
		}

		AggregateSynchronizationFilter aggregateFilter(filters);
		TimeSynchronizationSamples samples{ test::CreateTimeSyncSampleWithTimeOffset(5) };

		// Act:
		aggregateFilter(samples, NodeAge(123));

		// Assert:
		EXPECT_EQ(2u, capturedReturnValues.size());
		EXPECT_EQ(std::vector<bool>({ false, true }), capturedReturnValues);
	}

	// endregion

	// region alpha trimming

	namespace {
		TimeSynchronizationSamples CreateSamplesWithIncreasingOffsets(int64_t count) {
			TimeSynchronizationSamples samples;
			for (auto i = 0; i < count; ++i)
				samples.emplace(test::CreateTimeSyncSampleWithTimeOffset(i));

			return samples;
		}

		TimeSynchronizationSamples ExtractSubset(const TimeSynchronizationSamples& samples, int64_t start, int64_t end) {
			TimeSynchronizationSamples extractedSamples;
			auto startIter = samples.cbegin();
			auto endIter = samples.cbegin();
			std::advance(startIter, start);
			std::advance(endIter, end);
			extractedSamples.insert(startIter, endIter);
			return extractedSamples;
		}
	}

	TEST(TEST_CLASS, FiltersOutSamplesOnBothEnds) {
		// Arrange: 10 samples
		AggregateSynchronizationFilter aggregateFilter({});
		auto samples = CreateSamplesWithIncreasingOffsets(10);
		auto expectedSamples = ExtractSubset(samples, 2, 8);

		// Act:
		aggregateFilter(samples, NodeAge());

		// Assert: Alpha = 0.4, 10 * 0.4 / 2 = 2 samples are discarded on each end
		EXPECT_EQ(6u, samples.size());
		EXPECT_EQ(expectedSamples, samples);
	}

	TEST(TEST_CLASS, DoesNotFilterOutSamplesWhenSamplesSetIsTooSmall) {
		// Arrange: 10 samples
		AggregateSynchronizationFilter aggregateFilter({});
		auto samples = CreateSamplesWithIncreasingOffsets(3);
		auto expectedSamples = ExtractSubset(samples, 0, 3);

		// Act:
		aggregateFilter(samples, NodeAge());

		// Assert: Alpha = 0.4, 3 * 0.4 / 2 = 0 samples are discarded on each end
		EXPECT_EQ(3u, samples.size());
		EXPECT_EQ(expectedSamples, samples);
	}

	TEST(TEST_CLASS, AlphaTrimmingIsLastOperation) {
		// Arrange:
		std::vector<SynchronizationFilter> filters;
		for (auto i = 0u; i < 3; ++i) {
			filters.push_back([](const auto& sample, auto) {
				return 0 == sample.timeOffsetToRemote();
			});
		}

		AggregateSynchronizationFilter aggregateFilter(filters);
		auto samples = CreateSamplesWithIncreasingOffsets(5);

		// Act:
		aggregateFilter(samples, NodeAge());

		// Assert:
		// if alpha trimming is done first, 5 * 0.4 / 2 = 1 sample is removed at both ends resulting in 3 remaining samples
		// if alpha trimming is done last, normal filters remove first sample, then 4 * 0.4 / 2 = 0 samples are removed by alpha trimming
		// resulting in 4 remaining samples
		EXPECT_EQ(4u, samples.size());
	}

	// endregion
}}}
