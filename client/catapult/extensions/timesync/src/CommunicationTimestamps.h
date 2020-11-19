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
#include "catapult/types.h"

namespace catapult { namespace timesync {

#pragma pack(push, 1)

	/// Represents the network timestamps for sending and receiving a time synchronization request / response.
	struct CommunicationTimestamps {
	public:
		/// Creates default communication timestamps.
		CommunicationTimestamps() = default;

		/// Creates communication timestamps around \a sendTimestamp and \a receiveTimestamp.
		CommunicationTimestamps(Timestamp sendTimestamp, Timestamp receiveTimestamp)
				: SendTimestamp(sendTimestamp)
				, ReceiveTimestamp(receiveTimestamp)
		{}

	public:
		/// Returns \c true if these communication timestamps are equal to \a rhs.
		constexpr bool operator==(const CommunicationTimestamps& rhs) const {
			return SendTimestamp == rhs.SendTimestamp && ReceiveTimestamp == rhs.ReceiveTimestamp;
		}

		/// Returns \c true if these communication timestamps are not equal to \a rhs.
		constexpr bool operator!=(const CommunicationTimestamps& rhs) const {
			return !(*this == rhs);
		}

	public:
		/// Time when the request/response was sent.
		Timestamp SendTimestamp;

		/// Time when the request/response was received.
		Timestamp ReceiveTimestamp;
	};

#pragma pack(pop)
}}
