#pragma once
#include "catapult/io/Stream.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Memory-based implementation of input and output stream.
	class MockMemoryStream final : public io::InputStream, public io::OutputStream {
	public:
		/// Creates a memory stream with \a name and \a buffer.
		MockMemoryStream(const std::string& name, std::vector<uint8_t>& buffer);

	public:
		void write(const RawBuffer& buffer) override;

		void read(const MutableRawBuffer& buffer) override;

		void flush() override;

	public:
		/// Returns number of times flush was invoked.
		size_t numFlushes() const;

		/// Returns read position.
		size_t position() const;

	private:
		std::string m_name;
		std::vector<uint8_t>& m_buffer;
		mutable size_t m_readPosition;
		size_t m_flushCount;
	};
}}
