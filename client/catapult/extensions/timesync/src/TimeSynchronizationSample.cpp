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

#include "TimeSynchronizationSample.h"

namespace catapult { namespace timesync {

	namespace {
		constexpr int64_t ToSigned(Timestamp timestamp) {
			return static_cast<int64_t>(timestamp.unwrap());
		}
	}

	TimeSynchronizationSample::TimeSynchronizationSample()
			: m_identityKey()
			, m_localTimestamps()
			, m_remoteTimestamps()
	{}

	TimeSynchronizationSample::TimeSynchronizationSample(
			const Key& identityKey,
			const CommunicationTimestamps& localTimestamps,
			const CommunicationTimestamps& remoteTimestamps)
			: m_identityKey(identityKey)
			, m_localTimestamps(localTimestamps)
			, m_remoteTimestamps(remoteTimestamps)
	{}

	const Key& TimeSynchronizationSample::identityKey() const {
		return m_identityKey;
	}

	const CommunicationTimestamps& TimeSynchronizationSample::localTimestamps() const {
		return m_localTimestamps;
	}

	const CommunicationTimestamps& TimeSynchronizationSample::remoteTimestamps() const {
		return m_remoteTimestamps;
	}

	utils::TimeSpan TimeSynchronizationSample::duration() const {
		return utils::TimeSpan::FromDifference(m_localTimestamps.ReceiveTimestamp, m_localTimestamps.SendTimestamp);
	}

	int64_t TimeSynchronizationSample::localDuration() const {
		return ToSigned(m_localTimestamps.ReceiveTimestamp) - ToSigned(m_localTimestamps.SendTimestamp);
	}

	int64_t TimeSynchronizationSample::remoteDuration() const {
		return ToSigned(m_remoteTimestamps.SendTimestamp) - ToSigned(m_remoteTimestamps.ReceiveTimestamp);
	}

	// S=Send, R=Receive
	// remote node   ---------R-------S------->
	//                       o         \    network
	//                      /           \    time
	//                     /             o
	// local node    -----S---------------R--->
	int64_t TimeSynchronizationSample::timeOffsetToRemote() const {
		auto roundtripTime = localDuration() - remoteDuration();
		return ToSigned(m_remoteTimestamps.ReceiveTimestamp) - ToSigned(m_localTimestamps.SendTimestamp) - roundtripTime / 2;
	}

	bool TimeSynchronizationSample::operator<(const TimeSynchronizationSample& rhs) const {
		auto lhsTimeOffsetToRemote = timeOffsetToRemote();
		auto rhsTimeOffsetToRemote = rhs.timeOffsetToRemote();

		// since different remotes can report the same time offset and we are collecting the samples in a set,
		// we need to distinguish between nodes using the public key
		if (lhsTimeOffsetToRemote == rhsTimeOffsetToRemote)
			return m_identityKey < rhs.m_identityKey;

		return lhsTimeOffsetToRemote < rhsTimeOffsetToRemote;
	}

	bool TimeSynchronizationSample::operator==(const TimeSynchronizationSample& rhs) const {
		return m_identityKey == rhs.m_identityKey
				&& m_localTimestamps == rhs.m_localTimestamps
				&& m_remoteTimestamps == rhs.m_remoteTimestamps;
	}

	bool TimeSynchronizationSample::operator!=(const TimeSynchronizationSample& rhs) const {
		return !(*this == rhs);
	}
}}
