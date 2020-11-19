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
#include "CommunicationTimestamps.h"
#include "catapult/ionet/Node.h"
#include "catapult/utils/TimeSpan.h"
#include <set>

namespace catapult { namespace timesync {

	/// Represents a sample in the time synchronization process.
	class TimeSynchronizationSample {
	public:
		/// Creates a default time synchronization sample.
		TimeSynchronizationSample();

		/// Creates a time synchronization sample around \a identityKey, \a localTimestamps and \a remoteTimestamps.
		/// \a localTimestamps (\a remoteTimestamps) are the timestamps at which the local (remote) node sent and received
		/// a request and a response.
		TimeSynchronizationSample(
				const Key& identityKey,
				const CommunicationTimestamps& localTimestamps,
				const CommunicationTimestamps& remoteTimestamps);

	public:
		/// Gets the identity key.
		const Key& identityKey() const;

		/// Gets the local timestamps.
		const CommunicationTimestamps& localTimestamps() const;

		/// Gets the remote timestamps.
		const CommunicationTimestamps& remoteTimestamps() const;

	public:
		/// Gets the duration of the complete cycle.
		utils::TimeSpan duration() const;

		/// Gets the local duration.
		int64_t localDuration() const;

		/// Gets the remote duration.
		int64_t remoteDuration() const;

		/// Gets the offset of the local node's network time relative to the remote node's network time.
		int64_t timeOffsetToRemote() const;

	public:
		/// Returns \c true if this time synchronization sample is less than \a rhs.
		bool operator<(const TimeSynchronizationSample& rhs) const;

		/// Returns \c true if this time synchronization sample is equal to \a rhs.
		bool operator==(const TimeSynchronizationSample& rhs) const;

		/// Returns \c true if this time synchronization sample is not equal to \a rhs.
		bool operator!=(const TimeSynchronizationSample& rhs) const;

	private:
		Key m_identityKey;
		CommunicationTimestamps m_localTimestamps;
		CommunicationTimestamps m_remoteTimestamps;
	};

	/// Ordered set of time synchronization samples.
	using TimeSynchronizationSamples = std::set<TimeSynchronizationSample>;
}}
