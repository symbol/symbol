#pragma once
#include <limits>

namespace catapult { namespace test {

	/// Sets \a value to a maximum value allowed by its type.
	template<typename T>
	void SetMaxValue(T& value) {
		value = std::numeric_limits<T>::max();
	}
}}
