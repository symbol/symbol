#pragma once
#include "Waits.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"

namespace catapult { namespace test {

	/// A boolean flag that is automatically set on destruction.
	class AutoSetFlag {
	public:
		/// Creates a flag.
		AutoSetFlag() : m_flag(false), m_numWaiters(0)
		{}

		/// Destroys the flag.
		~AutoSetFlag() {
			set(); // unblock any waiting threads
		}

	public:
		/// Sets the flag.
		void set() {
			if (m_flag)
				return;

			CATAPULT_LOG(debug) << "setting auto set flag";
			m_flag = true;
		}

		/// Returns a value indicating whether the flag is set or not.
		bool isSet() const {
			return m_flag;
		}

		/// Waits for the flag to be set.
		void wait() const {
			auto guard = utils::MakeIncrementDecrementGuard(m_numWaiters);
			CATAPULT_LOG(debug) << "waiting for auto set flag (" << m_numWaiters << " waiters)";
			WAIT_FOR(m_flag);
		}

		/// Gets the number of threads waiting.
		size_t numWaiters() const {
			return m_numWaiters;
		}

	private:
		std::atomic_bool m_flag;
		mutable std::atomic<size_t> m_numWaiters;
	};
}}
