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

#include "RateMonitor.h"

namespace catapult { namespace ionet {

	RateMonitor::RateMonitor(
			const RateMonitorSettings& settings,
			const supplier<Timestamp>& timeSupplier,
			const action& rateExceededHandler)
			: m_settings(settings)
			, m_timeSupplier(timeSupplier)
			, m_rateExceededHandler(rateExceededHandler)
	{}

	size_t RateMonitor::bucketsSize() const {
		return m_buckets.size();
	}

	utils::FileSize RateMonitor::totalSize() const {
		uint64_t totalSize = 0;
		for (const auto& bucket : m_buckets)
			totalSize += bucket.TotalSize;

		return utils::FileSize::FromBytes(totalSize);
	}

	void RateMonitor::accept(uint32_t size) {
		// 1. prune all buckets that end before minRelevantTimestamp
		//   (buckets that have any overlap with minRelevantTimestamp are preserved)
		auto time = m_timeSupplier();
		auto minRelevantTimestamp = utils::SubtractNonNegative(
				time,
				utils::TimeSpan::FromMilliseconds((m_settings.NumBuckets - 1) * m_settings.BucketDuration.millis()));

		while (!m_buckets.empty() && endTime(m_buckets.front()) < minRelevantTimestamp)
			m_buckets.pop_front();

		// 2. add size observation
		add(time, size);

		// 3. check total size for violation
		if (totalSize() > m_settings.MaxTotalSize)
			m_rateExceededHandler();
	}

	void RateMonitor::add(Timestamp time, uint32_t size) {
		if (!m_buckets.empty()) {
			auto& currentBucket = m_buckets.back();
			if (currentBucket.StartTime <= time && time < endTime(currentBucket)) {
				currentBucket.TotalSize += size;
				return;
			}
		}

		m_buckets.push_back({ time, size });
	}

	Timestamp RateMonitor::endTime(const Bucket& bucket) const {
		return bucket.StartTime + m_settings.BucketDuration;
	}
}}
