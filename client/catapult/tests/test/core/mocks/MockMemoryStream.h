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
