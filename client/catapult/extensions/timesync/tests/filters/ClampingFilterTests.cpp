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
#include "timesync/src/filters/filter_constants.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync { namespace filters {

#define TEST_CLASS ClampingFilterTests

	namespace {
		int64_t ToInt64(const utils::TimeSpan& timeSpan) {
			return static_cast<int64_t>(timeSpan.millis());
		}
	}

	TEST(TEST_CLASS, FiltersOutSamplesWithIntolerableTimeOffsetToRemote) {
		// Arrange:
		auto filter = CreateClampingFilter();

		// Act + Assert:
		for (int64_t offset : { 1, 2, 10, 1000 }) {
			EXPECT_TRUE(filter(test::CreateTimeSyncSampleWithTimeOffset(ToInt64(Tolerated_Deviation_Start) + offset), NodeAge()));
			EXPECT_TRUE(filter(test::CreateTimeSyncSampleWithTimeOffset(-ToInt64(Tolerated_Deviation_Start) - offset), NodeAge()));
		}
	}

	TEST(TEST_CLASS, DoesNotFilterOutSamplesWithTolerableTimeOffsetToRemote) {
		// Arrange:
		auto filter = CreateClampingFilter();

		// Act + Assert:
		for (int64_t offset : { 0, 1, 2, 10, 1000 }) {
			EXPECT_FALSE(filter(test::CreateTimeSyncSampleWithTimeOffset(ToInt64(Tolerated_Deviation_Start) - offset), NodeAge()));
			EXPECT_FALSE(filter(test::CreateTimeSyncSampleWithTimeOffset(-ToInt64(Tolerated_Deviation_Start) + offset), NodeAge()));
		}
	}

	TEST(TEST_CLASS, FiltersHaveExpectedBehaviorBeforeDecay) {
		// Arrange:
		auto filter = CreateClampingFilter();
		TimeSynchronizationSamples samples;
		for (int64_t timeOffset : { 60'000, 80'000, 270'000, 1'700'000, 7'200'000 })
			samples.emplace(test::CreateTimeSyncSampleWithTimeOffset(timeOffset));

		// Act + Assert: no sample is filtered out before the decay starts
		// tolerated = exp(-0.3 * 0) * 120 * 60'000 = 7'200'000
		for (int64_t rawNodeAge : { 0ll, 1ll, Start_Decay_After_Round - 1ll, Start_Decay_After_Round + 0ll }) {
			for (const auto& sample : samples)
				EXPECT_FALSE(filter(sample, NodeAge(rawNodeAge))) << "filtering with node age " << rawNodeAge;
		}
	}

	TEST(TEST_CLASS, FiltersHaveExpectedBehaviorDuringDecay) {
		// Arrange:
		auto filter = CreateClampingFilter();
		TimeSynchronizationSamples samples;
		for (int64_t timeOffset : { 60'000, 80'000, 270'000, 1'700'000, 7'200'000 })
			samples.emplace(test::CreateTimeSyncSampleWithTimeOffset(timeOffset));

		// Act + Assert: note that a single sample is filtered out each round
		// tolerated = exp(-0.3 * 1) * 120 * 60'000 = 5'333'891
		// tolerated = exp(-0.3 * 5) * 120 * 60'000 = 1'606'537
		// tolerated = exp(-0.3 * 11) * 120 * 60'000 = 265'558
		// tolerated = exp(-0.3 * 15) * 120 * 60'000 = 79'984
		auto i = 0u;
		for (int64_t offset : { 1, 5, 11, 15 }) {
			auto j = 0u;
			for (const auto& sample : samples) {
				auto nodeAge = NodeAge(Start_Decay_After_Round + offset);
				auto message = "filtering sample " + std::to_string(j) + " with node age " + std::to_string(nodeAge.unwrap());
				if (j < 4 - i)
					EXPECT_FALSE(filter(sample, nodeAge)) << message;
				else
					EXPECT_TRUE(filter(sample, nodeAge)) << message;

				++j;
			}

			++i;
		}
	}

	TEST(TEST_CLASS, FiltersHaveExpectedBehaviorAfterDecay) {
		// Arrange:
		auto filter = CreateClampingFilter();
		TimeSynchronizationSamples samples;
		for (int64_t timeOffset : { 60'000, 80'000, 270'000, 1'700'000, 7'200'000 })
			samples.emplace(test::CreateTimeSyncSampleWithTimeOffset(timeOffset));

		// Act + Assert: only first sample is not filtered out
		// tolerated = max(exp(-0.3 * 16) * 120 * 60'000, Tolerated_Deviation_Minimum) = Tolerated_Deviation_Minimum
		for (int64_t offset : { 16, 17, 100, 1000 }) {
			auto i = 0u;
			for (const auto& sample : samples) {
				auto nodeAge = NodeAge(Start_Decay_After_Round + offset);
				auto message = "filtering sample " + std::to_string(i) + " with node age " + std::to_string(nodeAge.unwrap());
				if (0 == i)
					EXPECT_FALSE(filter(sample, nodeAge)) << message;
				else
					EXPECT_TRUE(filter(sample, nodeAge)) << message;

				++i;
			}
		}
	}
}}}
