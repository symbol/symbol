#include "DisruptorElement.h"
#include <ostream>

namespace catapult { namespace disruptor {

	std::ostream& operator<<(std::ostream& out, const DisruptorElement& element) {
		out << "element " << element.id() << " (" << element.input() << ")";
		return out;
	}
}}
