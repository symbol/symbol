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

#include "timesync/src/CommunicationTimestamps.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync {

#define TEST_CLASS CommunicationTimestampsTests

	TEST(TEST_CLASS, CanCreateDefaultCommunicationTimestamps) {
		// Act:
		CommunicationTimestamps timestamps;

		// Assert:
		EXPECT_EQ(Timestamp(0), timestamps.SendTimestamp);
		EXPECT_EQ(Timestamp(0), timestamps.ReceiveTimestamp);
	}

	TEST(TEST_CLASS, CanCreateCommunicationTimestamps) {
		// Act:
		CommunicationTimestamps timestamps(Timestamp(123), Timestamp(234));

		// Assert:
		EXPECT_EQ(Timestamp(123), timestamps.SendTimestamp);
		EXPECT_EQ(Timestamp(234), timestamps.ReceiveTimestamp);
	}

	// region equality operators

	namespace {
		const char* Default_Key = "default";

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}

		std::unordered_map<std::string, CommunicationTimestamps> GenerateEqualityInstanceMap() {
			return {
				{ Default_Key, CommunicationTimestamps(Timestamp(123), Timestamp(234)) },
				{ "copy", CommunicationTimestamps(Timestamp(123), Timestamp(234)) },
				{ "diff-send", CommunicationTimestamps(Timestamp(125), Timestamp(234)) },
				{ "diff-receive", CommunicationTimestamps(Timestamp(123), Timestamp(235)) }
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
