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
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/functions.h"
#include <deque>

namespace catapult { namespace ionet {

	/// Rate monitor settings.
	struct RateMonitorSettings {
		/// Number of buckets.
		uint16_t NumBuckets;

		/// Bucket duration.
		utils::TimeSpan BucketDuration;

		/// Maximum size allowed during full monitoring period.
		utils::FileSize MaxTotalSize;
	};

	/// Buckets and monitors data rates.
	class RateMonitor {
	public:
		/// Creates a monitor around \a settings, \a timeSupplier and \a rateExceededHandler.
		RateMonitor(const RateMonitorSettings& settings, const supplier<Timestamp>& timeSupplier, const action& rateExceededHandler);

	public:
		/// Gets the current number of buckets.
		size_t bucketsSize() const;

		/// Gets the total tracked size.
		utils::FileSize totalSize() const;

	public:
		/// Registers data of specified \a size at current time.
		void accept(uint32_t size);

	private:
		struct Bucket {
			Timestamp StartTime;
			uint64_t TotalSize;
		};

	private:
		void add(Timestamp time, uint32_t size);

		Timestamp endTime(const Bucket& bucket) const;

	private:
		RateMonitorSettings m_settings;
		supplier<Timestamp> m_timeSupplier;
		action m_rateExceededHandler;

		std::deque<Bucket> m_buckets;
	};
}}
