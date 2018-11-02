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

#include "RawFile.h"
#include "catapult/exceptions.h"
#include <memory>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

namespace catapult { namespace io {

	namespace {
		static const size_t Invalid_Size = static_cast<size_t>(-1);
		static const int Invalid_Descriptor = -1;
		static const int Read_Error = -1;
		static const int Write_Error = -1;

		static const char* Error_Open = "couldn't open the file";
		static const char* Error_Size = "couldn't determine file size";
		static const char* Error_Write = "couldn't write to file";
		static const char* Error_Read = "couldn't read from file";
		static const char* Error_Seek = "couldn't seek in file";
		static const char* Error_Desc = "invalid file descriptor";

#ifdef _MSC_VER
		constexpr auto Flag_Read_Only = _O_RDONLY;
		constexpr auto Flag_Read_Write = _O_RDWR;
		constexpr auto New_File_Create_Truncate_Flags = _O_CREAT | _O_TRUNC;
		constexpr auto New_File_Create_Flags = _O_CREAT;
		constexpr auto New_File_Permissions = _S_IWRITE | _S_IREAD;
		constexpr auto File_Binary_Flag = _O_BINARY;
		constexpr auto File_Locking_Exclusive = _SH_DENYRW;
		constexpr auto File_Locking_Shared_Read = _SH_DENYWR;
		constexpr auto File_Locking_None = _SH_DENYNO;

		constexpr auto close = ::_close;
		constexpr auto write = ::_write;
		constexpr auto read = ::_read;
		constexpr auto lseek = ::_lseeki64;
		constexpr auto fstat = ::_fstati64;
		using StatStruct = struct ::_stat64;

		template<typename TSize>
		inline unsigned int CastToDataSize(TSize size) {
			return static_cast<unsigned int>(size);
		}

		inline int open(const char* name, int flags, int lockingFlags, int permissions) {
			int fd;
			if (0 != _sopen_s(&fd, name, flags, lockingFlags, permissions))
				return Invalid_Descriptor;

			return fd;
		}
#else
		constexpr auto Flag_Read_Only = O_RDONLY;
		constexpr auto Flag_Read_Write = O_RDWR;
		constexpr auto New_File_Create_Truncate_Flags = O_CREAT | O_TRUNC;
		constexpr auto New_File_Create_Flags = O_CREAT;
		constexpr auto New_File_Permissions = S_IWUSR | S_IRUSR;
		constexpr auto File_Binary_Flag = 0;
		constexpr auto File_Locking_Exclusive = LOCK_NB | LOCK_EX;
		constexpr auto File_Locking_Shared_Read = LOCK_NB | LOCK_SH;
		constexpr auto File_Locking_None = LOCK_NB | LOCK_SH;
		using StatStruct = struct stat;

		template<typename TSize>
		inline size_t CastToDataSize(TSize size) {
			return static_cast<size_t>(size);
		}

		inline int open(const char* name, int flags, int lockingFlags, int permissions) {
			int fd = ::open(name, flags, permissions);
			if (Invalid_Descriptor == fd)
				return Invalid_Descriptor;

			if (-1 == ::flock(fd, lockingFlags)) {
				::close(fd);
				fd = Invalid_Descriptor;
			}

			return fd;
		}

		inline void close(int fd) {
			::flock(fd, LOCK_UN);
			::close(fd);
		}
#endif

		void nemClose(int fd) {
			close(fd);
		}

		template<typename TIoOperation, typename TBuffer>
		size_t ProcessInBlocks(TIoOperation ioOperation, int ioErrorCode, int fd, TBuffer& buffer) {
			auto* pData = buffer.pData;
			auto size = buffer.Size;
			size_t dataProcessed = 0;
			while (size > 0) {
				auto bytesToProcess = std::min<size_t>(0x40'00'00'00, size);
				auto ioResult = ioOperation(fd, pData, CastToDataSize(bytesToProcess));
				if (ioErrorCode == ioResult)
					return Invalid_Size;

				if (0 == ioResult)
					break;

				auto ioProcessed = CastToDataSize(ioResult);
				dataProcessed += ioProcessed;
				pData += ioProcessed;
				size -= ioProcessed;
			}

			return dataProcessed;
		}

		size_t nemWrite(int fd, const RawBuffer& data) {
			return ProcessInBlocks(write, Write_Error, fd, data);
		}

		size_t nemRead(int fd, const MutableRawBuffer& data) {
			return ProcessInBlocks(read, Read_Error, fd, data);
		}

		bool nemSeekSet(int fd, int64_t offset) {
			auto r = lseek(fd, offset, SEEK_SET);
			return offset == r;
		}

		bool nemFileSize(int fd, uint64_t& fileSize) {
			StatStruct st;
			fileSize = 0;
			if (0 != fstat(fd, &st))
				return false;

			fileSize = static_cast<uint64_t>(st.st_size);
			return true;
		}

		bool nemOpen(int& fd, const char* name, OpenMode mode, LockMode lockMode) {
			int flags = mode == OpenMode::Read_Only ? Flag_Read_Only : Flag_Read_Write;
			int createFlag = mode == OpenMode::Read_Write
					? New_File_Create_Truncate_Flags
					: (mode == OpenMode::Read_Append ? New_File_Create_Flags : 0);
			int lockingFlags = LockMode::File == lockMode
					? ((flags & Flag_Read_Write) ? File_Locking_Exclusive : File_Locking_Shared_Read)
					: File_Locking_None;

			fd = open(name, File_Binary_Flag | flags | createFlag, lockingFlags, New_File_Permissions);
			return Invalid_Descriptor != fd;
		}
	}

// note that this macro can only be used within RawFile member functions
#define CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(MESSAGE) \
	do { \
		CATAPULT_LOG(error) << MESSAGE << " " << m_pathname << (!m_fd.isValid() ? " (invalid)" : ""); \
		CATAPULT_THROW_FILE_IO_ERROR(MESSAGE); \
	} while (false)

	RawFile::RawFile(const std::string& pathname, OpenMode mode, LockMode lockMode)
			: m_pathname(pathname)
			, m_fd(Invalid_Descriptor)
			, m_fileSize(0)
			, m_position(0) {
		if (!nemOpen(m_fd.rawRef(), m_pathname.c_str(), mode, lockMode))
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Open);

		if (!nemFileSize(m_fd.raw(), m_fileSize))
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Size);
	}

	RawFile::FileDescriptorHolder::FileDescriptorHolder(int fd) : m_fd(fd)
	{}

	RawFile::FileDescriptorHolder::FileDescriptorHolder(FileDescriptorHolder&& rhs) : m_fd(rhs.m_fd) {
		rhs.m_fd = Invalid_Descriptor;
	}

	RawFile::FileDescriptorHolder::~FileDescriptorHolder() {
		if (Invalid_Descriptor != m_fd)
			nemClose(m_fd);

		m_fd = Invalid_Descriptor;
	}

	bool RawFile::FileDescriptorHolder::isValid() const {
		return Invalid_Descriptor != m_fd;
	}

	int RawFile::FileDescriptorHolder::raw() const {
		if (!isValid())
			CATAPULT_THROW_FILE_IO_ERROR(Error_Desc);

		return m_fd;
	}

	int& RawFile::FileDescriptorHolder::rawRef() {
		return m_fd;
	}

	void RawFile::write(const RawBuffer& dataBuffer) {
		if (dataBuffer.Size != nemWrite(m_fd.raw(), dataBuffer))
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Write);

		m_position += dataBuffer.Size;
		m_fileSize = std::max(m_fileSize, m_position);
	}

	void RawFile::read(const MutableRawBuffer& dataBuffer) {
		if (dataBuffer.Size != nemRead(m_fd.raw(), dataBuffer))
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Read);

		m_position += dataBuffer.Size;
	}

	void RawFile::seek(uint64_t position) {
		// Although low-level api allows seek outside the file, we won't allow
		// it. If we'll need it we'll add resize() and/or truncate() methods.
		if (position > size())
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Seek);

		if (!nemSeekSet(m_fd.raw(), static_cast<int64_t>(position)))
			CATAPULT_THROW_AND_LOG_RAW_FILE_ERROR(Error_Seek);

		m_position = position;
	}

	uint64_t RawFile::size() const {
		return m_fileSize;
	}

	uint64_t RawFile::position() const {
		return m_position;
	}
}}
