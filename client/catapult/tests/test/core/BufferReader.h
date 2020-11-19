/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/types.h"

namespace catapult { namespace test {

	/// Simple wrapper around a byte buffer that reads data by copying.
	/// \note This is intended to simplify testing serialized buffers that contain unaligned data.
	class BufferReader {
	public:
		/// Creates a reader around \a buffer.
		explicit BufferReader(const RawBuffer& buffer)
				: m_buffer(buffer)
				, m_offset(0)
		{}

	public:
		/// Gets the current offset.
		size_t position() const {
			return m_offset;
		}

		/// Gets a const pointer to the data at the current position.
		const uint8_t* data() const {
			return &m_buffer.pData[m_offset];
		}

	public:
		/// Reads a value and advances.
		template<typename TValue>
		TValue read() {
			TValue value;
			readInto(value);

			m_offset += sizeof(TValue);
			return value;
		}

		/// Advances \a count bytes without reading.
		void advance(size_t count) {
			m_offset += count;
		}

	private:
		template<typename TValue, typename TTag>
		void readInto(utils::BaseValue<TValue, TTag>& value) {
			TValue rawValue;
			readInto(rawValue);
			value = utils::BaseValue<TValue, TTag>(rawValue);
		}

		template<typename TTag>
		void readInto(utils::ByteArray<TTag>& byteArray) {
			std::memcpy(byteArray.data(), data(), utils::ByteArray<TTag>::Size);
		}

		template<typename TValue>
		void readInto(TValue& value) {
			std::memcpy(&value, data(), sizeof(TValue));
		}

	private:
		RawBuffer m_buffer;
		size_t m_offset;
	};
}}
