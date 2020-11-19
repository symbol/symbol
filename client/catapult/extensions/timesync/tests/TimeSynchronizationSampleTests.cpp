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

#include "timesync/src/TimeSynchronizationSample.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync {

#define TEST_CLASS TimeSynchronizationSampleTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultTimeSynchronizationSample) {
		// Act:
		TimeSynchronizationSample sample;

		// Assert:
		EXPECT_EQ(Key(), sample.identityKey());
		EXPECT_EQ(Timestamp(), sample.localTimestamps().SendTimestamp);
		EXPECT_EQ(Timestamp(), sample.localTimestamps().ReceiveTimestamp);
		EXPECT_EQ(Timestamp(), sample.remoteTimestamps().SendTimestamp);
		EXPECT_EQ(Timestamp(), sample.remoteTimestamps().ReceiveTimestamp);
	}

	TEST(TEST_CLASS, CanCreateTimeSynchronizationSample) {
		// Act:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		TimeSynchronizationSample sample(
				identityKey,
				test::CreateCommunicationTimestamps(12, 23),
				test::CreateCommunicationTimestamps(34, 45));

		// Assert:
		EXPECT_EQ(identityKey, sample.identityKey());
		EXPECT_EQ(Timestamp(12), sample.localTimestamps().SendTimestamp);
		EXPECT_EQ(Timestamp(23), sample.localTimestamps().ReceiveTimestamp);
		EXPECT_EQ(Timestamp(34), sample.remoteTimestamps().SendTimestamp);
		EXPECT_EQ(Timestamp(45), sample.remoteTimestamps().ReceiveTimestamp);
	}

	// endregion

	// region duration related

	TEST(TEST_CLASS, DurationIsCalculatedCorrectly) {
		// Arrange:
		auto sample1 = test::CreateSample(5, 17, 25, 23);
		auto sample2 = test::CreateSample(0, 31, 45, 45);
		auto sample3 = test::CreateSample(30, 30, 15, 13);

		// Act + Assert:
		EXPECT_EQ(utils::TimeSpan::FromMilliseconds(12), sample1.duration());
		EXPECT_EQ(utils::TimeSpan::FromMilliseconds(31), sample2.duration());
		EXPECT_EQ(utils::TimeSpan::FromMilliseconds(0), sample3.duration());
	}

	TEST(TEST_CLASS, LocalDurationIsCalculatedCorrectly) {
		// Arrange:
		auto sample1 = test::CreateSample(5, 17, 25, 23);
		auto sample2 = test::CreateSample(12, 3, 45, 45);
		auto sample3 = test::CreateSample(30, 30, 15, 13);

		// Act + Assert:
		EXPECT_EQ(12, sample1.localDuration());
		EXPECT_EQ(-9, sample2.localDuration());
		EXPECT_EQ(0, sample3.localDuration());
	}

	TEST(TEST_CLASS, RemoteDurationIsCalculatedCorrectly) {
		// Arrange:
		auto sample1 = test::CreateSample(5, 17, 25, 23);
		auto sample2 = test::CreateSample(0, 31, 45, 49);
		auto sample3 = test::CreateSample(30, 30, 15, 15);

		// Act + Assert:
		EXPECT_EQ(2, sample1.remoteDuration());
		EXPECT_EQ(-4, sample2.remoteDuration());
		EXPECT_EQ(0, sample3.remoteDuration());
	}

	// endregion

	// region timeOffsetToRemote

	TEST(TEST_CLASS, TimeOffsetToRemoteIsCalculatedCorrectly) {
		// Arrange:
		auto sample1 = test::CreateSample(5, 17, 25, 23);
		auto sample2 = test::CreateSample(8, 12, 45, 45);
		auto sample3 = test::CreateSample(37, 43, 15, 13);
		auto sample4 = test::CreateSample(0, 19, 15, 13);

		// Act + Assert:
		EXPECT_EQ(13, sample1.timeOffsetToRemote());
		EXPECT_EQ(35, sample2.timeOffsetToRemote());
		EXPECT_EQ(-26, sample3.timeOffsetToRemote());
		EXPECT_EQ(5, sample4.timeOffsetToRemote());
	}

	// endregion

	// region operator<

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Arrange:
		auto node1 = test::CreateNamedNode({ { 1 } }, "alice");
		auto node2 = test::CreateNamedNode({ { 2 } }, "bob");
		std::vector<TimeSynchronizationSample> samples{
			test::CreateSample(node1, 37, 43, 15, 13), // time offset -26
			test::CreateSample(node2, 37, 43, 15, 13), // time offset -26
			test::CreateSample(node1, 5, 17, 25, 23), // time offset 13
			test::CreateSample(node1, 8, 12, 45, 45) // time offset 35
		};

		// Assert:
		test::AssertLessThanOperatorForEqualValues(test::CreateSample(node1, 8, 12, 45, 45), test::CreateSample(node1, 8, 12, 45, 45));
		test::AssertOperatorBehaviorForIncreasingValues(samples, std::less<>(), [](const auto& sample) {
			std::ostringstream out;
			out
					<< "local (" << sample.localTimestamps().SendTimestamp << "," << sample.localTimestamps().ReceiveTimestamp << ")"
					<< ", remote (" << sample.remoteTimestamps().SendTimestamp << "," << sample.remoteTimestamps().ReceiveTimestamp << ")";
			return out.str();
		});
	}

	// endregion

	// region equality operators

	namespace {
		const char* Default_Key = "default";

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}

		std::unordered_map<std::string, TimeSynchronizationSample> GenerateEqualityInstanceMap() {
			auto node = test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "alice");
			return {
				{ Default_Key, test::CreateSample(node, 37, 43, 15, 13) },
				{ "copy", test::CreateSample(node, 37, 43, 15, 13) },
				{ "diff-node", test::CreateSample(37, 43, 15, 13) },
				{ "diff-local-timestamps", test::CreateSample(node, 27, 43, 15, 13) },
				{ "diff-remote-timestamps", test::CreateSample(node, 37, 43, 25, 13) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
