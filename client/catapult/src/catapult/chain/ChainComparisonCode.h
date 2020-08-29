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
#include <stdint.h>

namespace catapult { namespace chain {

	/// Chain comparison code flag that is set if the code indicates an out of sync remote node.
	constexpr uint32_t Remote_Is_Out_Of_Sync_Flag = 0x40000000;

	/// Chain comparison code flag that is set if the code indicates an evil remote node.
	constexpr uint32_t Remote_Is_Evil_Flag = 0x80000000;

#define CHAIN_COMPARISON_CODE_LIST \
	/* Remote and local nodes reported equal chain scores. */ \
	ENUM_VALUE(Remote_Reported_Equal_Chain_Score, 1) \
	\
	/* Remote node reported a lower chain score than the local node. */ \
	ENUM_VALUE(Remote_Reported_Lower_Chain_Score, 2) \
	\
	/* Local height changed during update operation. */ \
	ENUM_VALUE(Local_Height_Updated, 3) \
	\
	/* Remote node is too far behind the local node. */ \
	ENUM_VALUE(Remote_Is_Too_Far_Behind, Remote_Is_Out_Of_Sync_Flag | 1) \
	\
	/* Remote node is not in sync with the local node. */ \
	ENUM_VALUE(Remote_Is_Not_Synced, Remote_Is_Out_Of_Sync_Flag | 2) \
	\
	/* Remote node returned too many hashes. */ \
	ENUM_VALUE(Remote_Returned_Too_Many_Hashes, Remote_Is_Evil_Flag | 1) \
	\
	/* Remote node is on a fork. */ \
	ENUM_VALUE(Remote_Is_Forked, Remote_Is_Evil_Flag | 2) \
	\
	/* Remote node lied about having a higher chain score. */ \
	ENUM_VALUE(Remote_Lied_About_Chain_Score, Remote_Is_Evil_Flag | 3)

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Possible chain comparison end states.
	enum class ChainComparisonCode : uint32_t {
		CHAIN_COMPARISON_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ChainComparisonCode value);

	/// Gets a value indicating whether or not \a code indicates that the remote node is out of sync.
	bool IsRemoteOutOfSync(ChainComparisonCode code);

	/// Gets a value indicating whether or not \a code indicates that the remote node is evil.
	bool IsRemoteEvil(ChainComparisonCode code);
}}
