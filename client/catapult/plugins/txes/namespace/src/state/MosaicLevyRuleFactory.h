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

#pragma once
#include "MosaicLevy.h"
#include <functional>
#include <vector>

namespace catapult { namespace model { struct Mosaic; } }

namespace catapult { namespace state {

	/// Available mosaic levy rule ids.
	enum class RuleId : uint8_t {
		/// Use lower bound fee.
		Constant,
		/// Use percentile fee.
		Percentile,
		/// Use the max(lower bound fee, percentile fee).
		Lower_Bounded_Percentile,
		/// Use min(upper bound fee, percentile fee).
		Upper_Bounded_Percentile,
		/// Use min(upper bound fee, max(lower bound fee, percentile fee)).
		Bounded_Percentile
	};

	/// A factory for creating mosaic levy rules.
	class MosaicLevyRuleFactory {
	public:
		/// Prototype for a mosaic levy rule creator.
		using MosaicLevyRuleCreator = std::function<MosaicLevyRule (Amount lowerBount, Amount upperBound, uint64_t percentile)>;

	public:
		/// Creates a mosaic levy rule factory.
		MosaicLevyRuleFactory();

	public:
		/// Gets the number of rule creation functions in the factory.
		size_t size() const {
			return m_rules.size();
		}

	public:
		/// Creates a mosaic levy rule around \a lowerBound, \a upperBound, \a percentile and \a ruleId.
		MosaicLevyRule createRule(Amount lowerBound, Amount upperBound, uint64_t percentile, RuleId ruleId);

	private:
		std::vector<MosaicLevyRuleCreator> m_rules;
	};
}}
