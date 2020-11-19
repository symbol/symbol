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

#include "MosaicRestrictionEvaluator.h"
#include "MosaicAddressRestriction.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace state {

	namespace {
		bool Evaluate(const MosaicGlobalRestriction::RestrictionRule& rule, uint64_t value, bool& isValidRule) {
			isValidRule = true;
			switch (rule.RestrictionType) {
			case model::MosaicRestrictionType::EQ:
				return rule.RestrictionValue == value;

			case model::MosaicRestrictionType::NE:
				return rule.RestrictionValue != value;

			case model::MosaicRestrictionType::LT:
				return rule.RestrictionValue > value;

			case model::MosaicRestrictionType::LE:
				return rule.RestrictionValue >= value;

			case model::MosaicRestrictionType::GT:
				return rule.RestrictionValue < value;

			case model::MosaicRestrictionType::GE:
				return rule.RestrictionValue <= value;

			default:
				break;
			}

			isValidRule = false;
			return false;
		}
	}

	bool EvaluateMosaicRestriction(const MosaicGlobalRestriction::RestrictionRule& rule, uint64_t value) {
		bool isValidRule;
		auto evaluateResult = Evaluate(rule, value, isValidRule);

		if (!isValidRule) {
			CATAPULT_LOG(error)
					<< "cannot evaluate mosaic restriction rule with unsupported type "
					<< static_cast<uint16_t>(rule.RestrictionType);
			return false;
		}

		// if unset value, only NE should match
		// otherwise, use result of evaluation
		return MosaicAddressRestriction::Sentinel_Removal_Value == value
				? model::MosaicRestrictionType::NE == rule.RestrictionType
				: evaluateResult;
	}
}}
