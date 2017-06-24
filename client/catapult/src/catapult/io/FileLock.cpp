#include "FileLock.h"
#include "catapult/utils/Logging.h"
#include <mutex>
#include <thread>
#include <fcntl.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

namespace catapult { namespace io {

	namespace {
		const int Invalid_Descriptor = -1;

#ifdef _MSC_VER
		constexpr auto NemClose = ::_close;

		int NemOpen(const std::string& lockFilePath) {
			constexpr auto Open_Flags = _O_RDONLY | _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY;

			int fd;
			return 0 == ::_sopen_s(&fd, lockFilePath.c_str(), Open_Flags, _SH_DENYRW, _S_IWRITE | _S_IREAD)
					? fd
					: Invalid_Descriptor;
		}

		void NemUnlink(const std::string&) {}
#else
		constexpr auto NemClose = ::close;

		bool TryLockFile(int fd) {
			if (0 == ::flock(fd, LOCK_NB | LOCK_EX))
				return true;

			::close(fd);
			return false;
		}

		int NemOpen(const std::string& lockFilePath) {
			int fd = ::open(lockFilePath.c_str(), O_RDONLY | O_CREAT | O_CLOEXEC | O_EXCL, 0);
			return (Invalid_Descriptor != fd) && TryLockFile(fd) ? fd : Invalid_Descriptor;
		}

		void NemUnlink(const std::string& lockFilePath) {
			::unlink(lockFilePath.c_str());
		}
#endif

		bool TryOpenLock(const std::string& lockFilePath, int& fd) {
			fd = NemOpen(lockFilePath);
			if (Invalid_Descriptor == fd) {
				CATAPULT_LOG(warning) << "LockOpen failed: " << errno;
				return false;
			}

			return true;
		}

		void DestroyLock(const std::string& lockFilePath, int& fd) {
			if (Invalid_Descriptor == fd)
				return;

			NemUnlink(lockFilePath);
			NemClose(fd);
			fd = Invalid_Descriptor;
		}
	}

	FileLock::FileLock(const std::string& lockFilePath)
			: m_lockFilePath(lockFilePath)
			, m_fd(Invalid_Descriptor)
	{}

	FileLock::~FileLock() {
		unlock();
	}

	void FileLock::lock() {
		while (!try_lock())
			std::this_thread::yield();
	}

	bool FileLock::try_lock() {
		if (!m_spinLock.try_lock())
			return false;

		bool isLockAcquired = (Invalid_Descriptor == m_fd) && TryOpenLock(m_lockFilePath, m_fd);
		m_spinLock.unlock();
		return isLockAcquired;
	}

	void FileLock::unlock() noexcept {
		std::lock_guard<utils::SpinLock> guard(m_spinLock);
		DestroyLock(m_lockFilePath, m_fd);
	}
}}
