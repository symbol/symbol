#pragma once
#include "IoTypes.h"

namespace catapult { namespace ionet {

	/// Context for enabling in-place data writes into a buffer.
	/// If commit is not called, the write is abandoned.
	class AppendContext {
	public:
		/// Creates an append context for appending \a size data to \a data.
		AppendContext(ByteBuffer& data, size_t size);

		/// Move constructor.
		AppendContext(AppendContext&& rhs);

		/// Destructor.
		~AppendContext();

	public:
		/// An asio buffer that can be written to in place.
		boost::asio::mutable_buffers_1 buffer();

		/// Commits the write to the underlying buffer.
		void commit(size_t size);

	private:
		void assertNotCommitted() const;

	private:
		ByteBuffer& m_data;
		size_t m_appendSize;
		size_t m_originalSize;
		bool m_isCommitted;
	};
}}
