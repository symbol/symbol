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
