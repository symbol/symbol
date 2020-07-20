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
#include <windows.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

namespace catapult { namespace io {

	namespace {
		// region constants

		constexpr size_t Invalid_Size = static_cast<size_t>(-1);
		constexpr int Invalid_Descriptor = -1;
		constexpr int Read_Error = -1;
		constexpr int Write_Error = -1;

		constexpr const char* Error_Open = "couldn't open the file";
		constexpr const char* Error_Size = "couldn't determine file size";
		constexpr const char* Error_Write = "couldn't write to file";
		constexpr const char* Error_Read = "couldn't read from file";
		constexpr const char* Error_Seek = "couldn't seek in file";
		constexpr const char* Error_Seek_Outside = "couldn't seek past end of file";
		constexpr const char* Error_Desc = "invalid file descriptor";
		constexpr const char* Error_Close = "couldn't close the file";

		// endregion

		// region FileOperationResult

		template<typename T>
		class FileOperationResult {
		public:
			FileOperationResult(bool isSuccess, T value)
					: IsSuccess(isSuccess)
					, Value(value)
			{}

		public:
			bool IsSuccess;
			T Value;

			int32_t ErrorCode;
			std::string Message;
		};

		template<typename T>
		FileOperationResult<T> MakeSuccessResult(T value) {
			return FileOperationResult<T>(true, value);
		}

		// endregion

		// region platform-dependent file io

#ifdef _MSC_VER
		constexpr auto Flag_Read_Only = _O_RDONLY;
		constexpr auto Flag_Read_Write = _O_RDWR;
		constexpr auto New_File_Create_Truncate_Flags = _O_CREAT | _O_TRUNC;
		constexpr auto New_File_Create_Flags = _O_CREAT;
		constexpr auto New_File_Permissions = _S_IWRITE | _S_IREAD;
		constexpr auto Open_Flags = _O_BINARY;
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

		template<typename T>
		FileOperationResult<T> MakeFailureResult(T value) {
			auto lastError = ::GetLastError();

			FileOperationResult<T> result(false, value);
			result.ErrorCode = static_cast<int32_t>(lastError);

			constexpr uint32_t Max_Message_Size = 256;
			result.Message.resize(Max_Message_Size);

			auto formatMessageFlags = FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
			auto messageSize = ::FormatMessage(formatMessageFlags, nullptr, lastError, 0, &result.Message[0], Max_Message_Size, nullptr);
			result.Message.resize(messageSize);
			return result;
		}

		inline FileOperationResult<int> open(int& fd, const char* name, int flags, int lockingFlags, int permissions) {
			auto result = _sopen_s(&fd, name, flags, lockingFlags, permissions);
			if (0 != result) {
				fd = Invalid_Descriptor;
				return MakeFailureResult(result);
			}

			return MakeSuccessResult(0);
		}
#else
		constexpr auto Flag_Read_Only = O_RDONLY;
		constexpr auto Flag_Read_Write = O_RDWR;
		constexpr auto New_File_Create_Truncate_Flags = O_CREAT | O_TRUNC;
		constexpr auto New_File_Create_Flags = O_CREAT;
		constexpr auto New_File_Permissions = S_IWUSR | S_IRUSR;
		constexpr auto Open_Flags = O_CLOEXEC;
		constexpr auto File_Locking_Exclusive = LOCK_NB | LOCK_EX;
		constexpr auto File_Locking_Shared_Read = LOCK_NB | LOCK_SH;
		constexpr auto File_Locking_None = 0;

		constexpr auto close = ::close; // ::close unlocks all files, so explicit flock is not needed
		using StatStruct = struct stat;

		template<typename TSize>
		inline size_t CastToDataSize(TSize size) {
			return static_cast<size_t>(size);
		}

		template<typename T>
		FileOperationResult<T> MakeFailureResult(T value) {
			auto lastError = errno;

			FileOperationResult<T> result(false, value);
			result.ErrorCode = lastError;
			result.Message = std::strerror(lastError);
			return result;
		}

		inline FileOperationResult<int> open(int& fd, const char* name, int flags, int lockingFlags, int permissions) {
			fd = ::open(name, flags, permissions);
			if (Invalid_Descriptor == fd)
				return MakeFailureResult(Invalid_Descriptor);

			if (0 != lockingFlags && -1 == ::flock(fd, lockingFlags)) {
				auto result = MakeFailureResult(-1);
				::close(fd);
				fd = Invalid_Descriptor;
				return result;
			}

			return MakeSuccessResult(0);
		}
#endif
		// endregion

		// region platform-independent nem file io wrappers

		FileOperationResult<bool> nemClose(int fd) {
			return -1 == close(fd) ? MakeFailureResult(false) : MakeSuccessResult(true);
		}

		template<typename TIoOperation, typename TBuffer>
		FileOperationResult<size_t> ProcessInBlocks(TIoOperation ioOperation, int ioErrorCode, int fd, TBuffer& buffer) {
			auto* pData = buffer.pData;
			auto size = buffer.Size;
			size_t numBytesProcessed = 0;
			while (size > 0) {
				auto numBytesToProcess = std::min<size_t>(0x40'00'00'00, size);
				auto ioResult = ioOperation(fd, pData, CastToDataSize(numBytesToProcess));
				if (ioErrorCode == ioResult)
					return MakeFailureResult(Invalid_Size);

				if (0 == ioResult)
					break;

				auto ioProcessed = CastToDataSize(ioResult);
				numBytesProcessed += ioProcessed;
				pData += ioProcessed;
				size -= ioProcessed;
			}

			return buffer.Size == numBytesProcessed ? MakeSuccessResult(numBytesProcessed) : MakeFailureResult(numBytesProcessed);
		}

		FileOperationResult<size_t> nemWrite(int fd, const RawBuffer& data) {
			return ProcessInBlocks(write, Write_Error, fd, data);
		}

		FileOperationResult<size_t> nemRead(int fd, const MutableRawBuffer& data) {
			return ProcessInBlocks(read, Read_Error, fd, data);
		}

		FileOperationResult<bool> nemSeekSet(int fd, int64_t offset) {
			return -1 == lseek(fd, offset, SEEK_SET) ? MakeFailureResult(false) : MakeSuccessResult(true);
		}

		FileOperationResult<bool> nemFileSize(int fd, uint64_t& fileSize) {
			StatStruct st;
			fileSize = 0;
			if (0 != fstat(fd, &st))
				return MakeFailureResult(false);

			fileSize = static_cast<uint64_t>(st.st_size);
			return MakeSuccessResult(true);
		}

		FileOperationResult<int> nemOpen(int& fd, const char* name, OpenMode mode, LockMode lockMode) {
			int flags = mode == OpenMode::Read_Only ? Flag_Read_Only : Flag_Read_Write;
			int createFlag = mode == OpenMode::Read_Write
					? New_File_Create_Truncate_Flags
					: (mode == OpenMode::Read_Append ? New_File_Create_Flags : 0);
			int lockingFlags = LockMode::File == lockMode
					? ((flags & Flag_Read_Write) ? File_Locking_Exclusive : File_Locking_Shared_Read)
					: File_Locking_None;

			return open(fd, name, Open_Flags | flags | createFlag, lockingFlags, New_File_Permissions);
		}

		// endregion
	}

	// region RawFile::FileDescriptorHolder

	RawFile::FileDescriptorHolder::FileDescriptorHolder(int fd) : m_fd(fd)
	{}

	RawFile::FileDescriptorHolder::FileDescriptorHolder(FileDescriptorHolder&& rhs) : m_fd(rhs.m_fd) {
		rhs.m_fd = Invalid_Descriptor;
	}

	RawFile::FileDescriptorHolder::~FileDescriptorHolder() {
		if (Invalid_Descriptor == m_fd)
			return;

		auto closeResult = nemClose(m_fd);
		if (closeResult.IsSuccess) {
			m_fd = Invalid_Descriptor;
			return;
		}

		CATAPULT_LOG(error)
				<< Error_Close << " " << m_fd << ": "
				<< closeResult.Message << " (" << closeResult.ErrorCode << ")";
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

	// endregion

	// region RawFile

// note that this macro can only be used within RawFile member functions
#define CATAPULT_CHECK_FILE_OPERATION_RESULT(MESSAGE, OPERATION_RESULT) \
	do { \
		if (!OPERATION_RESULT.IsSuccess) { \
			CATAPULT_LOG(error) \
					<< MESSAGE << " " << m_pathname << ": " \
					<< OPERATION_RESULT.Message << " (" << OPERATION_RESULT.ErrorCode << ")"; \
			CATAPULT_THROW_FILE_IO_ERROR(MESSAGE); \
		} \
	} while (false)

	RawFile::RawFile(const std::string& pathname, OpenMode mode, LockMode lockMode)
			: m_pathname(pathname)
			, m_fd(Invalid_Descriptor)
			, m_fileSize(0)
			, m_position(0) {
		auto openResult = nemOpen(m_fd.rawRef(), m_pathname.c_str(), mode, lockMode);
		CATAPULT_CHECK_FILE_OPERATION_RESULT(Error_Open, openResult);

		auto fileSizeResult = nemFileSize(m_fd.raw(), m_fileSize);
		CATAPULT_CHECK_FILE_OPERATION_RESULT(Error_Size, fileSizeResult);
	}

	void RawFile::write(const RawBuffer& dataBuffer) {
		auto writeResult = nemWrite(m_fd.raw(), dataBuffer);
		CATAPULT_CHECK_FILE_OPERATION_RESULT(Error_Write, writeResult);

		m_position += writeResult.Value;
		m_fileSize = std::max(m_fileSize, m_position);
	}

	void RawFile::read(const MutableRawBuffer& dataBuffer) {
		auto readResult = nemRead(m_fd.raw(), dataBuffer);
		CATAPULT_CHECK_FILE_OPERATION_RESULT(Error_Read, readResult);

		m_position += readResult.Value;
	}

	void RawFile::seek(uint64_t position) {
		// constrain seek to inside the file even though low-level api allows seek outside the file
		// if needed, such behavior is better suited for resize and/or truncate methods
		if (position > size()) {
			CATAPULT_LOG(error) << Error_Seek_Outside << " " << m_pathname;
			CATAPULT_THROW_FILE_IO_ERROR(Error_Seek_Outside);
		}

		auto seekSetResult = nemSeekSet(m_fd.raw(), static_cast<int64_t>(position));
		CATAPULT_CHECK_FILE_OPERATION_RESULT(Error_Seek, seekSetResult);

		m_position = position;
	}

	uint64_t RawFile::size() const {
		return m_fileSize;
	}

	uint64_t RawFile::position() const {
		return m_position;
	}

	// endregion
}}
