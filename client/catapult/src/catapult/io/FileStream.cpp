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

#include "FileStream.h"

namespace catapult { namespace io {

	FileStream::FileStream(RawFile&& rawFile) : m_rawFile(std::move(rawFile))
	{}

	void FileStream::write(const RawBuffer& buffer) {
		m_rawFile.write(buffer);
	}

	void FileStream::flush()
	{}

	bool FileStream::eof() const {
		return m_rawFile.position() == m_rawFile.size();
	}

	void FileStream::read(const MutableRawBuffer& buffer) {
		m_rawFile.read(buffer);
	}

	void FileStream::seek(uint64_t position) {
		m_rawFile.seek(position);
	}

	uint64_t FileStream::position() const {
		return m_rawFile.position();
	}
}}
