#pragma once
#include <stddef.h>

namespace catapult { namespace local {

	/// Value that is returned when a statistics source is \c nullptr.
	constexpr size_t Sentinel_Stats_Value = static_cast<size_t>(-1);

	/// Gets a value from \a pSource using \a getter or \c Sentinel_Stats_Value when \a pSource is \c nullptr.
	template<typename TSourcePointer, typename THandler>
	inline size_t GetStatsValue(const TSourcePointer& pSource, THandler getter) {
		return nullptr == pSource ? Sentinel_Stats_Value : ((*pSource).*getter)();
	}
}}
