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

#pragma once
#include "AppendContext.h"
#include "IoTypes.h"
#include "PacketExtractor.h"
#include "PacketSocketOptions.h"

namespace catapult { namespace ionet {

	/// Buffer for storing working data.
	class WorkingBuffer {
	public:
		/// Creates an empty working buffer around \a options.
		explicit WorkingBuffer(const PacketSocketOptions& options);

	public:
		/// Gets a const iterator to the beginning of the buffer
		inline auto begin() const {
			return m_data.cbegin();
		}

		/// Gets a const iterator to the end of the buffer.
		inline auto end() const {
			return m_data.cend();
		}

		/// Gets the size of the buffer.
		inline auto size() const {
			return m_data.size();
		}

		/// Gets a const pointer to the raw buffer.
		inline auto data() const {
			return m_data.data();
		}

		/// Gets the capacity of the raw buffer.
		inline auto capacity() const {
			return m_data.capacity();
		}

	public:
		/// Appends \a byte to the end of the working buffer.
		void append(uint8_t byte);

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
