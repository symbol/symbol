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
#include "RawFile.h"
#include "SeekableStream.h"

namespace catapult { namespace io {

	/// File-based implementation of input and output stream.
	class FileStream : public SeekableStream {
	public:
		/// Creates a file stream around \a rawFile.
		explicit FileStream(RawFile&& rawFile);

	public:
		void write(const RawBuffer& buffer) override;
		void flush() override;

	public:
		bool eof() const override;
		void read(const MutableRawBuffer& buffer) override;

	public:
		void seek(uint64_t position) override;
		uint64_t position() const override;

	private:
		RawFile m_rawFile;
	};
}}
