#pragma once

namespace catapult {

#ifdef _MSC_VER

#define CPP14_CONSTEXPR __forceinline
#define CATAPULT_INLINE __forceinline

#else

#define CPP14_CONSTEXPR constexpr
#define CATAPULT_INLINE inline

#endif

}
