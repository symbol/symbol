#include "FileSize.h"
#include "IntegerMath.h"
#include <iostream>

namespace catapult { namespace utils {

	namespace {
		class OutputWriter {
		public:
			OutputWriter(std::ostream& out) :
				m_out(out),
				m_hasOutput(false)
			{}

		public:
			void write(uint64_t value, const char* postfix) {
				if (m_hasOutput)
					m_out << " ";

				m_out << value << postfix;
				m_hasOutput = true;
			}

			void writeNonZero(uint64_t value, const char* postfix) {
				if (0 == value)
					return;

				write(value, postfix);
			}

			void writeIfEmpty(uint64_t value, const char* postfix) {
				if (m_hasOutput)
					return;

				write(value, postfix);
			}

		private:
			std::ostream& m_out;
			bool m_hasOutput;
		};
	}

	std::ostream& operator<<(std::ostream& out, const FileSize& fileSize) {
		// calculate components
		auto totalBytes = fileSize.bytes();
		auto bytes = DivideAndGetRemainder<uint64_t>(totalBytes, 1024);
		auto kilobytes = DivideAndGetRemainder<uint64_t>(totalBytes, 1024);
		auto megabytes = totalBytes;

		// output as [0MB][ ][0KB][ ][0B]
		OutputWriter writer(out);
		writer.writeNonZero(megabytes, "MB");
		writer.writeNonZero(kilobytes, "KB");
		writer.writeNonZero(bytes, "B");
		writer.writeIfEmpty(0, "B"); // if file size is 0, output "0B"
		return out;
	}
}}
