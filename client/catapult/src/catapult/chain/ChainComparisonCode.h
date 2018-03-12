#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace chain {

	/// Chain comparison code flag that is set if the code indicates an evil remote node.
	constexpr uint32_t Remote_Is_Evil_Flag = 0x80000000;

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

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Possible chain comparison end states.
	enum class ChainComparisonCode : uint32_t {
		CHAIN_COMPARISON_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ChainComparisonCode value);

	/// Gets a value indicating whether or not \a code indicates that the remote node is evil.
	bool IsRemoteEvil(ChainComparisonCode code);
}}
