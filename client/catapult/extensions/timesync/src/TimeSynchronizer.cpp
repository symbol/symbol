/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "TimeSynchronizer.h"
#include "constants.h"
#include "timesync/src/filters/filter_constants.h"
#include "catapult/cache_core/AccountStateCacheView.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/utils/Functional.h"
#include <cmath>

namespace catapult { namespace timesync {

	namespace {
		constexpr utils::LogLevel MapToLogLevel(int64_t warningThresholdMillis, int64_t offset) {
			return warningThresholdMillis > offset ? utils::LogLevel::trace : utils::LogLevel::warning;
		}

		double GetCoupling(NodeAge nodeAge) {
			auto ageToUse = static_cast<double>(std::max<int64_t>(nodeAge.unwrap() - filters::Start_Decay_After_Round, 0));
			return std::max(std::exp(-Coupling_Decay_Strength * ageToUse) * Coupling_Start, Coupling_Minimum);
		}
	}

	TimeSynchronizer::TimeSynchronizer(
			const filters::AggregateSynchronizationFilter& filter,
			Importance totalChainImportance,
			int64_t warningThresholdMillis)
			: m_filter(filter)
			, m_totalChainImportance(totalChainImportance)
			, m_warningThresholdMillis(warningThresholdMillis)
	{}

	TimeOffset TimeSynchronizer::calculateTimeOffset(
			const cache::AccountStateCacheView& accountStateCacheView,
			Height height,
			TimeSynchronizationSamples&& samples,
			NodeAge nodeAge) {
		m_filter(samples, nodeAge);
		if (samples.empty()) {
			CATAPULT_LOG(warning) << "no synchronization samples available to calculate network time";
			return TimeOffset(0);
		}

		cache::ImportanceView importanceView(accountStateCacheView.asReadOnly());
		auto cumulativeImportance = sumImportances(importanceView, height, samples);
		if (0 == cumulativeImportance) {
			CATAPULT_LOG(warning) << "cannot calculate network time, cumulativeImportance is zero";
			return TimeOffset(0);
		}

		auto highValueAddressesSize = accountStateCacheView.highValueAccounts().addresses().size();
		auto viewPercentage = utils::to_ratio(samples.size(), highValueAddressesSize);
		auto importancePercentage = utils::to_ratio(cumulativeImportance, m_totalChainImportance.unwrap());
		auto scaling = importancePercentage > viewPercentage ? 1.0 / importancePercentage : 1.0 / viewPercentage;
		auto sum = sumScaledOffsets(importanceView, height, samples, scaling);
		return TimeOffset(static_cast<int64_t>(GetCoupling(nodeAge) * sum));
	}

	Importance::ValueType TimeSynchronizer::sumImportances(
			const cache::ImportanceView& importanceView,
			Height height,
			const TimeSynchronizationSamples& samples) {
		return utils::Sum(samples, [&importanceView, height](const auto& sample) {
			return importanceView.getAccountImportanceOrDefault(sample.identityKey(), height).unwrap();
		});
	}

	double TimeSynchronizer::sumScaledOffsets(
			const cache::ImportanceView& importanceView,
			Height height,
			const TimeSynchronizationSamples& samples,
			double scaling) {
		auto totalChainImportance = m_totalChainImportance.unwrap();
		auto warningThresholdMillis = m_warningThresholdMillis;
		return utils::Sum(samples, [&importanceView, height, scaling, totalChainImportance, warningThresholdMillis](const auto& sample) {
			int64_t offset = sample.timeOffsetToRemote();
			CATAPULT_LOG_LEVEL(MapToLogLevel(warningThresholdMillis, offset))
					<< sample.identityKey() << ": network time offset to local node is " << offset << "ms";

			auto importance = importanceView.getAccountImportanceOrDefault(sample.identityKey(), height);
			auto importancePercentage = utils::to_ratio(importance.unwrap(), totalChainImportance);
			return scaling * static_cast<double>(offset) * importancePercentage;
		});
	}
}}
