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

#pragma once
#include "timesync/src/TimeSynchronizationSample.h"
#include "catapult/ionet/Node.h"
#include <vector>

namespace catapult { namespace test {

	/// Creates communication timestamps around \a sendTime and \a receiveTime.
	timesync::CommunicationTimestamps CreateCommunicationTimestamps(int64_t sendTime, int64_t receiveTime);

	/// Creates a time synchronization sample around \a node, \a localSendTime, \a localReceiveTime, \a remoteSendTime
	/// and \a remoteReceiveTime.
	timesync::TimeSynchronizationSample CreateSample(
			const ionet::Node& node,
			int64_t localSendTime,
			int64_t localReceiveTime,
			int64_t remoteSendTime,
			int64_t remoteReceiveTime);

	/// Creates a time synchronization sample around \a localSendTime, \a localReceiveTime, \a remoteSendTime and \a remoteReceiveTime.
	timesync::TimeSynchronizationSample CreateSample(
			int64_t localSendTime,
			int64_t localReceiveTime,
			int64_t remoteSendTime,
			int64_t remoteReceiveTime);

	/// Creates a time synchronization sample with a given \a duration.
	timesync::TimeSynchronizationSample CreateTimeSyncSampleWithDuration(int64_t duration);

	/// Creates a time synchronization sample with a given \a timeOffset.
	timesync::TimeSynchronizationSample CreateTimeSyncSampleWithTimeOffset(int64_t timeOffset);

	/// Creates \a count time synchronization samples with increasing time offsets starting with \a initialTimeOffset.
	timesync::TimeSynchronizationSamples CreateTimeSyncSamplesWithIncreasingTimeOffset(int64_t initialTimeOffset, int64_t count);

	/// Extracts public keys from \a samples.
	template<typename TSamplesContainer>
	std::vector<Key> ExtractKeys(const TSamplesContainer& samples) {
		std::vector<Key> keys;
		for (const auto& sample : samples)
			keys.push_back(sample.identityKey());

		return keys;
	}
}}
