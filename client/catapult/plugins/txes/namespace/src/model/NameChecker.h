#pragma once
#include <stddef.h>
#include <stdint.h>

namespace catapult { namespace model {

	/// Returns \c true if \a pName with size \a nameSize points to a valid name.
	bool IsValidName(const uint8_t* pName, size_t nameSize);
}}
