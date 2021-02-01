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
#include <string>

namespace catapult { namespace io {

	/// Defines mode of opening the file.
	enum class OpenMode {
		/// Open file in read-only mode.
		Read_Only,

		/// Open file for both reading and writing. Removes the file if it exists.
		Read_Write,

		/// Open file for both reading and writing.
		Read_Append
	};

	/// Defines locking mode for the file.
	enum class LockMode {
		/// Don't use any locking.
		None,

		/// Use file-based locking.
		File
	};

	/// Wrapper for low-level i/o operations on files.
	class RawFile final : public utils::MoveOnly {
	public:
		/// Opens file pointed to by \a pathname either for reading or both for reading and writing
		/// as specified by \a mode. Optionally file locking can be disabled via \a lockMode.
		RawFile(const std::string& pathname, OpenMode mode, LockMode lockMode = LockMode::File);

		/// Move construct a raw file from \a rhs.
		RawFile(RawFile&& rhs) = default;

		/// Disallow move-assign.
		RawFile& operator=(RawFile&& rhs) = delete;

	public:
		/// Gets the size of the file.
		uint64_t size() const;

		/// Gets the position in the file.
		uint64_t position() const;

	public:
		/// Reads data from the file into \a dataBuffer
		/// Throws catapult_file_io_error exception if requested amount of data could not be read.
		void read(const MutableRawBuffer& dataBuffer);

		/// Writes data pointed to by \a dataBuffer to the file.
		/// If proper amount of data could not be written catapult_file_io_error exception will be thrown.
		void write(const RawBuffer& dataBuffer);

		/// Seeks to given absolute position.
		/// Throws catapult_file_io_error exception if seek has failed.
		void seek(uint64_t position);

		/// Truncates the file at its current position.
		void truncate();

	private:
		class FileDescriptorHolder final {
		public:
			explicit FileDescriptorHolder(int fd);
			FileDescriptorHolder(FileDescriptorHolder&& rhs);
			FileDescriptorHolder& operator=(FileDescriptorHolder&& rhs) = delete;
			~FileDescriptorHolder();

			bool isValid() const;

			int raw() const;
			int& rawRef();

		private:
			int m_fd;
		};

		const std::string m_pathname;
		FileDescriptorHolder m_fd;
		uint64_t m_fileSize;
		uint64_t m_position;
	};
}}
