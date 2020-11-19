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
#include "TimeSpan.h"

namespace catapult { namespace utils {

	/// Network time, which is set to zero in the nemesis block and increases as time advances.
	class NetworkTime {
	public:
		/// Creates network time around \a epochAdjustment, which is relative to unix timestamp epoch (1970-01-01 00:00:00 UTC).
		explicit NetworkTime(const utils::TimeSpan& epochAdjustment);

	public:
		/// Gets the current network time.
		Timestamp now() const;

		/// Given a unix \a timestamp, returns the corresponding network timestamp that the unix timestamp represents.
		Timestamp toNetworkTime(const Timestamp& timestamp) const;

		/// Given a network \a timestamp, returns the corresponding unix timestamp that the network timestamp represents.
		Timestamp toUnixTime(const Timestamp& timestamp) const;

	private:
		utils::TimeSpan m_epochAdjustment;
	};
}}
