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
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace ionet {

#define NODE_INTERACTION_RESULT_CODE_LIST \
	/* No experience. */ \
	ENUM_VALUE(None) \
	\
	/* Experience was neutral. */ \
	ENUM_VALUE(Neutral) \
	\
	/* Experience was good. */ \
	ENUM_VALUE(Success) \
	\
	/* Experience was bad. */ \
	ENUM_VALUE(Failure)

#define ENUM_VALUE(LABEL) LABEL,
	/// Possible node interaction result codes.
	enum class NodeInteractionResultCode {
		NODE_INTERACTION_RESULT_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NodeInteractionResultCode value);
}}
