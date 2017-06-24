#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace chain {

/// Possible node interaction results.
#define NODE_INTERACTION_RESULT_LIST \
	/* The experience was neutral. */ \
	ENUM_VALUE(Neutral) \
	\
	/* The experience was good. */ \
	ENUM_VALUE(Success) \
	\
	/* The experience was bad. */ \
	ENUM_VALUE(Failure) \

#define DECLARE_ENUM NodeInteractionResult
#define ENUM_LIST NODE_INTERACTION_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM
}}
