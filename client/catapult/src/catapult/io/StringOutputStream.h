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

namespace catapult { namespace io {

	/// String output stream.
	class StringOutputStream : public OutputStream {
	public:
		/// Creates string output stream with reserved \a capacity.
		explicit StringOutputStream(size_t capacity) {
			m_output.reserve(capacity);
		}

	public:
		void write(const RawBuffer& buffer) override {
			m_output.append(reinterpret_cast<const char*>(buffer.pData), buffer.Size);
		}

		void flush() override
		{}

	public:
		/// Gets the underlying string.
		const std::string& str() const {
			return m_output;
		}

	private:
		std::string m_output;
	};
}}
