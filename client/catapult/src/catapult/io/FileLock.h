#pragma once
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/SpinLock.h"
#include <string>

namespace catapult { namespace io {

	/// Lock based on an underlying lock file.
	class FileLock : public utils::NonCopyable {
	public:
		/// Creates a lock file inside \a directoryPath.
		explicit FileLock(const std::string& directoryPath);

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
		int m_fd;
	};
}}
