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

#include "MosaicLevyRuleFactory.h"
#include "catapult/model/AccountInfo.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	namespace {
		constexpr Amount Min_Amount(0);
		constexpr Amount Max_Amount(std::numeric_limits<Amount::ValueType>::max());

		constexpr auto Percentile(uint64_t percentile, Amount amount) {
			return Amount(percentile * amount.unwrap() / 10'000);
		}

		auto BoundedPercentile(Amount lowerBound, Amount upperBound, uint64_t percentile) {
			// note that the mosaic passed in is composed of
			// - the levy mosaic id
			// - the mosaic amount of a transfer
			return [lowerBound, upperBound, percentile](const model::Mosaic& mosaic) {
				auto value = std::min(upperBound, std::max(lowerBound, Percentile(percentile, mosaic.Amount)));
				return model::Mosaic{ mosaic.MosaicId, value };
			};
		}
	}

	MosaicLevyRuleFactory::MosaicLevyRuleFactory() {
		// Constant
		m_rules.push_back([](Amount lowerBound, Amount, uint64_t) {
			return BoundedPercentile(lowerBound, lowerBound, 0);
		});

		// Percentile
		m_rules.push_back([](Amount, Amount, uint64_t percentile) {
			return BoundedPercentile(Min_Amount, Max_Amount, percentile);
		});

		// Lower_Bounded_Percentile
		m_rules.push_back([](Amount lowerBound, Amount, uint64_t percentile) {
			return BoundedPercentile(lowerBound, Max_Amount, percentile);
		});

		// Upper_Bounded_Percentile
		m_rules.push_back([](Amount, Amount upperBound, uint64_t percentile) {
			return BoundedPercentile(Min_Amount, upperBound, percentile);
		});

		// Bounded_Percentile
		m_rules.push_back([](Amount lowerBound, Amount upperBound, uint64_t percentile) {
			return BoundedPercentile(lowerBound, upperBound, percentile);
		});
	}

	MosaicLevyRule MosaicLevyRuleFactory::createRule(Amount lowerBound, Amount upperBound, uint64_t percentile, RuleId ruleId) {
		auto id = utils::to_underlying_type(ruleId);
		if (m_rules.size() <= id)
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown rule with id", static_cast<uint32_t>(id));

		return m_rules[id](lowerBound, upperBound, percentile);
	}
}}
