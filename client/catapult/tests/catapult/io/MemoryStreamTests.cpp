#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/catapult/io/utils/StreamTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

	namespace {
		class MemoryStreamContext {
		public:
			explicit MemoryStreamContext(const char* name) : m_name(name)
			{}

			auto outputStream() {
				return std::make_unique<mocks::MemoryStream>(m_name, m_buffer);
			}

			auto inputStream() {
				return std::make_unique<mocks::MemoryStream>(m_name, m_buffer);
			}

		private:
			std::string m_name;
			std::vector<uint8_t> m_buffer;
		};
	}

	DEFINE_STREAM_TESTS(MemoryStream)
}}
