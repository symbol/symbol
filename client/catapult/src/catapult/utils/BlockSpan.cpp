#include "BlockSpan.h"
#include "IntegerMath.h"
#include <iostream>

namespace catapult { namespace utils {

	std::ostream& operator<<(std::ostream& out, const BlockSpan& blockSpan) {
		// calculate components
		auto totalHours = blockSpan.hours();
		auto hours = DivideAndGetRemainder<uint64_t>(totalHours, 24);
		auto days = totalHours;

		// output as 0d 0h
		out << days << "d " << hours << "h";
		return out;
	}
}}
