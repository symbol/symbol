#include "ChainComparisonCode.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace chain {

#define DEFINE_ENUM ChainComparisonCode
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST CHAIN_COMPARISON_CODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM

	bool IsRemoteEvil(ChainComparisonCode code) {
		return 0 != (Remote_Is_Evil_Flag & utils::to_underlying_type(code));
	}
}}
