#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace chain {

	/// Chain comparison code flag that is set if the code indicates an evil remote node.
	constexpr uint32_t Remote_Is_Evil_Flag = 0x80000000;

/// Possible chain comparison end states.
#define CHAIN_COMPARISON_CODE_LIST \
	/* The remote node is too far behind the local node. */ \
	ENUM_VALUE(Remote_Is_Too_Far_Behind, 3) \
	\
	/* The remote node is not in sync with the local node. */ \
	ENUM_VALUE(Remote_Is_Not_Synced, 4) \
	\
	/* The remote and local nodes reported equal chain scores. */ \
	ENUM_VALUE(Remote_Reported_Equal_Chain_Score, 5) \
	\
	/* The remote node reported a lower chain score than the local node. */ \
	ENUM_VALUE(Remote_Reported_Lower_Chain_Score, 6) \
	\
	/* The remote node returned too many hashes. */ \
	ENUM_VALUE(Remote_Returned_Too_Many_Hashes, Remote_Is_Evil_Flag | 2) \
	\
	/* The remote node is on a fork. */ \
	ENUM_VALUE(Remote_Is_Forked, Remote_Is_Evil_Flag | 3) \
	\
	/* The remote node lied about having a higher chain score. */ \
	ENUM_VALUE(Remote_Lied_About_Chain_Score, Remote_Is_Evil_Flag | 4)

#define DECLARE_ENUM ChainComparisonCode
#define EXPLICIT_VALUE_ENUM
#define EXPLICIT_TYPE_ENUM uint32_t
#define ENUM_LIST CHAIN_COMPARISON_CODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_TYPE_ENUM
#undef EXPLICIT_VALUE_ENUM
#undef DECLARE_ENUM

	/// Gets a value indicating whether or not \a code indicates that the remote node is evil.
	bool IsRemoteEvil(ChainComparisonCode code);
}}
