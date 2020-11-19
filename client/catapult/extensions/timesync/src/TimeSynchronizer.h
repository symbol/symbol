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
#include "constants.h"
#include "types.h"
#include "timesync/src/filters/AggregateSynchronizationFilter.h"

namespace catapult {
	namespace cache {
		class AccountStateCacheView;
		class ImportanceView;
	}
}

namespace catapult { namespace timesync {

	/// Time synchronizer that synchronizes local time with the network.
	class TimeSynchronizer {
	public:
		/// Creates a time synchronizer around \a filter, \a totalChainImportance and \a warningThresholdMillis.
		TimeSynchronizer(
				const filters::AggregateSynchronizationFilter& filter,
				Importance totalChainImportance,
				int64_t warningThresholdMillis = Warning_Threshold_Millis);

	public:
		/// Calculates a time offset from \a samples using \a accountStateCacheView, \a height and \a nodeAge.
		TimeOffset calculateTimeOffset(
				const cache::AccountStateCacheView& accountStateCacheView,
				Height height,
				TimeSynchronizationSamples&& samples,
				NodeAge nodeAge);

	private:
		Importance::ValueType sumImportances(
				const cache::ImportanceView& importanceView,
				Height height,
				const TimeSynchronizationSamples& samples);

		double sumScaledOffsets(
				const cache::ImportanceView& importanceView,
				Height height,
				const TimeSynchronizationSamples& samples,
				double scaling);
	private:
		filters::AggregateSynchronizationFilter m_filter;
		Importance m_totalChainImportance;
		int64_t m_warningThresholdMillis;
	};
}}
