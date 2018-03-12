#pragma once
#include "AppendContext.h"
#include "IoTypes.h"
#include "PacketExtractor.h"
#include "PacketSocketOptions.h"

namespace catapult { namespace ionet {

	/// A buffer for storing working data.
	class WorkingBuffer {
	public:
		/// Creates an empty working buffer around \a options.
		explicit WorkingBuffer(const PacketSocketOptions& options);

	public:
		/// Returns a const iterator to the beginning of the buffer
		inline auto begin() const {
			return m_data.cbegin();
		}

		/// Returns a const iterator to the end of the buffer.
		inline auto end() const {
			return m_data.cend();
		}

		/// Returns the size of the buffer.
		inline auto size() const {
			return m_data.size();
		}

		/// Returns a const pointer to the raw buffer.
		inline auto data() const {
			return m_data.data();
		}

		/// Returns the capacity of the raw buffer.
		inline auto capacity() const {
			return m_data.capacity();
		}

	public:
		/// Creates an append context that can be used to append data to the working buffer.
		AppendContext prepareAppend();

		/// Creates a packet extractor that can be used to extract packets from the working buffer.
		PacketExtractor preparePacketExtractor();

	private:
		void checkMemoryUsage();

	private:
		PacketSocketOptions m_options;
		ByteBuffer m_data;
		size_t m_numDataSizeSamples;
		size_t m_maxDataSize;
	};
}}
