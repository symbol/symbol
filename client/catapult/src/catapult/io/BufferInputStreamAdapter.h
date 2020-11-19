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
#include "Stream.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/exceptions.h"
#include <sstream>

namespace catapult { namespace io {

	/// Adapt a typed buffer to be used as an input stream.
	template<typename TContainer>
	class BufferInputStreamAdapter : public InputStream {
	public:
		/// Creates an input stream around \a input.
		explicit BufferInputStreamAdapter(const TContainer& input)
				: m_input(input)
				, m_position(0)
		{}

	public:
		/// Gets the read position.
		size_t position() const {
			return m_position;
		}

	public:
		bool eof() const override {
			RawBuffer input(m_input);
			return m_position == input.Size;
		}

		void read(const MutableRawBuffer& buffer) override {
			RawBuffer input(m_input);
			if (buffer.Size + m_position > input.Size) {
				std::ostringstream out;
				out
						<< "BufferInputStreamAdapter invalid read (buffer-size = " << buffer.Size
						<< ", position = " << m_position
						<< ", input-size = " << input.Size << ")";
				CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
			}

			utils::memcpy_cond(buffer.pData, input.pData + m_position, buffer.Size);
			m_position += buffer.Size;
		}

	private:
		const TContainer& m_input;
		size_t m_position;
	};
}}
