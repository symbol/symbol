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
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/SpinLock.h"
#include <string>

namespace catapult { namespace io {

	/// Lock based on an underlying lock file.
	class FileLock : public utils::NonCopyable {
	public:
		/// Implementation dependent type of file descriptor.
		/// \note Windows version is using void* instead of HANDLE to avoid including windows.h
#ifdef _MSC_VER
		using FdType = void*;
#else
		using FdType = int;
#endif

	public:
		/// Creates a lock file with path \a lockFilePath.
		explicit FileLock(const std::string& lockFilePath);

		/// Releases the lock file.
		~FileLock();

	public:
		/// Blocks until a lock can be obtained for the current execution agent.
		void lock();

		/// Attempts to acquire the lock for the current execution agent without blocking.
		bool try_lock();

		/// Releases the lock held by the execution agent. Throws no exceptions.
		void unlock() noexcept;

	private:
		std::string m_lockFilePath;
		utils::SpinLock m_spinLock;
		FdType m_fd;
	};
}}
