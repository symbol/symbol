#pragma once
#include "SpinLock.h"
#include "catapult/exceptions.h"
#include <mutex>
#include <thread>
#include <unordered_set>

namespace catapult { namespace utils {

	/// Exception class that is thrown when reader reentrancy is detected.
	class reader_reentrancy_error : public catapult_runtime_error {
	public:
		using catapult_runtime_error::catapult_runtime_error;
	};

	/// A reentrancy check reader notification policy.
	class ReentrancyCheckReaderNotificationPolicy {
	public:
		/// A reader was acquried by the current thread.
		void readerAcquired() {
			executeSynchronized([this](auto id) {
				if (m_threadIds.cend() != m_threadIds.find(id))
					CATAPULT_THROW_AND_LOG_1(reader_reentrancy_error, "reader reentrancy detected", id);

				m_threadIds.insert(id);
			});
		}

		/// A reader was released by the current thread.
		void readerReleased() {
			executeSynchronized([this](auto id) {
				m_threadIds.erase(id);
			});
		}

	private:
		template<typename TAction>
		void executeSynchronized(TAction action) {
			auto id = std::this_thread::get_id();
			std::lock_guard<SpinLock> lock(m_mutex);

			action(id);
		}

	private:
		SpinLock m_mutex;
		std::unordered_set<std::thread::id> m_threadIds;
	};
}}
