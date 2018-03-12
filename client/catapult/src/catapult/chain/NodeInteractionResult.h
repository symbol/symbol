#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace chain {

#define NODE_INTERACTION_RESULT_LIST \
	/* There was no experience. */ \
	ENUM_VALUE(None) \
	\
	/* The experience was neutral. */ \
	ENUM_VALUE(Neutral) \
	\
	/* The experience was good. */ \
	ENUM_VALUE(Success) \
	\
	/* The experience was bad. */ \
	ENUM_VALUE(Failure) \

#define ENUM_VALUE(LABEL) LABEL,
	/// Possible node interaction results.
	enum class NodeInteractionResult {
		NODE_INTERACTION_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NodeInteractionResult value);
}}
