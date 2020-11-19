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

#include "TimeSynchronizationTestUtils.h"
#include "timesync/src/filters/filter_constants.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Default_Start_Time = static_cast<int64_t>(utils::TimeSpan::FromMinutes(240).millis());
	}

	timesync::CommunicationTimestamps CreateCommunicationTimestamps(int64_t sendTime, int64_t receiveTime) {
		return timesync::CommunicationTimestamps(
				Timestamp(static_cast<Timestamp::ValueType>(sendTime)),
				Timestamp(static_cast<Timestamp::ValueType>(receiveTime)));
	}

	timesync::TimeSynchronizationSample CreateSample(
			const ionet::Node& node,
			int64_t localSendTime,
			int64_t localReceiveTime,
			int64_t remoteSendTime,
			int64_t remoteReceiveTime) {
		return timesync::TimeSynchronizationSample(
				node.identity().PublicKey,
				CreateCommunicationTimestamps(localSendTime, localReceiveTime),
				CreateCommunicationTimestamps(remoteSendTime, remoteReceiveTime));
	}

	timesync::TimeSynchronizationSample CreateSample(
			int64_t localSendTime,
			int64_t localReceiveTime,
			int64_t remoteSendTime,
			int64_t remoteReceiveTime) {
		auto node = test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "alice");
		return CreateSample(node, localSendTime, localReceiveTime, remoteSendTime, remoteReceiveTime);
	}

	timesync::TimeSynchronizationSample CreateTimeSyncSampleWithDuration(int64_t duration) {
		return timesync::TimeSynchronizationSample(
				test::GenerateRandomByteArray<Key>(),
				CreateCommunicationTimestamps(0, duration),
				CreateCommunicationTimestamps(duration / 2, duration / 2));
	}

	timesync::TimeSynchronizationSample CreateTimeSyncSampleWithTimeOffset(int64_t timeOffset) {
		// remote timestamps have 0 duration to make it more readable
		return timesync::TimeSynchronizationSample(
				test::GenerateRandomByteArray<Key>(),
				CreateCommunicationTimestamps(0, 2 * Default_Start_Time + 100),
				CreateCommunicationTimestamps(Default_Start_Time + 50 + timeOffset, Default_Start_Time + 50 + timeOffset));
	}

	timesync::TimeSynchronizationSamples CreateTimeSyncSamplesWithIncreasingTimeOffset(int64_t initialTimeOffset, int64_t count) {
		timesync::TimeSynchronizationSamples samples;
		for (auto i = 0; i < count; ++i)
			samples.insert(CreateTimeSyncSampleWithTimeOffset(initialTimeOffset + i));

		return samples;
	}
}}
