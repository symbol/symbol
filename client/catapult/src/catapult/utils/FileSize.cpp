/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
