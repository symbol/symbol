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
#include <iosfwd>

namespace catapult { namespace chain {

#define ROUND_MESSAGE_AGGREGATOR_ADD_RESULT_LIST \
	/* Message was rejected because it has an invalid number of hashes. */ \
	ENUM_VALUE(Failure_Invalid_Hashes) \
	\
	/* Message was rejected because it has an invalid point. */ \
	ENUM_VALUE(Failure_Invalid_Point) \
	\
	/* Message was rejected because it has an invalid height. */ \
	ENUM_VALUE(Failure_Invalid_Height) \
	\
	/* Message was rejected because it failed processing. */ \
	ENUM_VALUE(Failure_Processing) \
	\
	/* Message was rejected because it conflicts with a previous message from the same sender. */ \
	ENUM_VALUE(Failure_Conflicting) \
	\
	/* Message was skipped because it has already been received. */ \
	ENUM_VALUE(Neutral_Redundant) \
	\
	/* Message was accepted as a prevote. */ \
	ENUM_VALUE(Success_Prevote) \
	\
	/* Message was accepted as a precommit. */ \
	ENUM_VALUE(Success_Precommit)

#define ENUM_VALUE(LABEL) LABEL,
	/// Round message aggregator add results.
	enum class RoundMessageAggregatorAddResult {
		ROUND_MESSAGE_AGGREGATOR_ADD_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, RoundMessageAggregatorAddResult value);
}}
