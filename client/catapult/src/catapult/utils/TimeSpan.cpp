#include "TimeSpan.h"
#include "IntegerMath.h"
#include <iomanip>
#include <iostream>

namespace catapult { namespace utils {

	namespace {
		class FormatterGuard {
		public:
			FormatterGuard(std::ostream& out) :
				m_out(out),
				m_flags(m_out.flags(std::ios::dec)),
				m_fill(m_out.fill('0'))
			{}

			~FormatterGuard() {
				m_out.flags(m_flags);
				m_out.fill(m_fill);
			}

		private:
			std::ostream& m_out;
			std::ios_base::fmtflags m_flags;
			char m_fill;
		};
	}

	std::ostream& operator<<(std::ostream& out, const TimeSpan& timeSpan) {
		// calculate components
		auto totalMillis = timeSpan.millis();
		auto millis = DivideAndGetRemainder<uint64_t>(totalMillis, 1000);
		auto seconds = DivideAndGetRemainder<uint64_t>(totalMillis, 60);
		auto minutes = DivideAndGetRemainder<uint64_t>(totalMillis, 60);
		auto hours = totalMillis;

		// output as 00:00:00[.000]
		FormatterGuard guard(out);
		out << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds;

		if (0 != millis)
			out << "." << std::setw(3) << millis;

		return out;
	}
}}
