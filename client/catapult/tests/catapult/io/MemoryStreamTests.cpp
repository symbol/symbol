#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/catapult/io/test/StreamTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

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

	DEFINE_STREAM_TESTS(MemoryStream)
}}
