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

#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/catapult/io/test/StreamTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS MemoryStreamTests

	namespace {
		class MemoryStreamContext {
		public:
			explicit MemoryStreamContext(const char* name) : m_name(name)
			{}

			auto outputStream() {
				return std::make_unique<mocks::MockMemoryStream>(m_name, m_buffer);
			}

			auto inputStream() {
				return std::make_unique<mocks::MockMemoryStream>(m_name, m_buffer);
			}

		private:
			std::string m_name;
			std::vector<uint8_t> m_buffer;
		};
	}

	DEFINE_STREAM_TESTS(MemoryStreamContext)
}}
