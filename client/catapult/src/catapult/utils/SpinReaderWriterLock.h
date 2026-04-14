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
#include "catapult/exceptions.h"
#include "catapult/functions.h"
#include <atomic>
#include <thread>

namespace catapult {
	namespace utils {
		template<typename TReaderNotificationPolicy>
		struct SpinReaderWriterLockTestAccessor;
	}
}

namespace catapult { namespace utils {

	/// Custom reader writer lock implemented by using an atomic that allows multiple readers and a single writer
	/// and prefers writers.
	/// \note
	/// - 32,768 max writers (pending + active combined)
	/// - 65,535 max readers
	template<typename TReaderNotificationPolicy>
	class BasicSpinReaderWriterLock : private TReaderNotificationPolicy {
	private:
		// Bit layout (32-bit atomic):
		// Bit 31:       [1 bit]  Active writer flag
		// Bits 30-16:   [15 bits] Writer count (pending + active)
		// Bits 15-0:    [16 bits] Active reader count

		static constexpr uint32_t Active_Writer_Flag = 0b10000000000000000000000000000000;

		// Mask for total writer count bits (pending + active)
		static constexpr uint32_t Writer_Count_Mask = 0b01111111111111110000000000000000;

		static constexpr uint32_t Reader_Mask = 0b00000000000000001111111111111111;
		static constexpr uint32_t Writer_Mask = Writer_Count_Mask | Active_Writer_Flag;

		static constexpr uint32_t Active_Reader_Increment = 0b00000000000000000000000000000001;

		// Increment for total writer count (pending + active)
		static constexpr uint32_t Writer_Count_Increment = 0b00000000000000010000000000000000;

		// For use in tests only
		friend struct SpinReaderWriterLockTestAccessor<TReaderNotificationPolicy>;

	private:
		// region YieldStepper

		class YieldStepper {
		private:
			static constexpr uint32_t MIN_WAIT = 0;
			static constexpr uint32_t MAX_WAIT = 256;
			static constexpr uint32_t NUM_ATTEMPTS_PER_WAIT = 100;
			static constexpr uint32_t MULTIPLIER = 4;

		public:
			YieldStepper()
					: m_waitMillis(MIN_WAIT)
					, m_numRemaining(NUM_ATTEMPTS_PER_WAIT)
			{}

		public:
			void yield() {
				if (0 == m_waitMillis)
					std::this_thread::yield();
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(m_waitMillis));

				if (0 == --m_numRemaining)
					moveToNextWait();
			}

		private:
			void moveToNextWait() {
				m_numRemaining = NUM_ATTEMPTS_PER_WAIT;
				if (MAX_WAIT <= m_waitMillis)
					return;

				m_waitMillis = 0 == m_waitMillis ? 1 : m_waitMillis * MULTIPLIER;
			}

		private:
			uint32_t m_waitMillis;
			uint32_t m_numRemaining;
		};

		// endregion

	private:
		// region LockGuard

		/// Base class for RAII lock guards.
		class LockGuard {
		protected:
			explicit LockGuard(const action& resetFunc)
					: m_resetFunc(resetFunc)
					, m_isMoved(false)
			{}

			~LockGuard() {
				if (m_isMoved)
					return;

				m_resetFunc();
			}

		public:
			LockGuard(LockGuard&& rhs) : m_resetFunc(rhs.m_resetFunc), m_isMoved(false) {
				rhs.m_isMoved = true;
			}

		private:
			action m_resetFunc;
			bool m_isMoved;
		};

		// endregion

	public:
		// region WriterLockGuard

		/// RAII writer lock guard.
		class WriterLockGuard : public LockGuard {
		public:
			/// Creates a guard around \a value.
			explicit WriterLockGuard(std::atomic<uint32_t>& value)
					: LockGuard([&value]() {
						// unset the active writer flag
						value.fetch_sub(Active_Writer_Flag + Writer_Count_Increment);
					})
			{}

			/// Creates a guard around \a value and \a isActive.
			/// \note This constructor is used when writer is created by promotion.
			WriterLockGuard(std::atomic<uint32_t>& value, bool& isActive)
					: LockGuard([&value, &isActive]() {
						// unset the active writer flag and change the writer to a reader
						value.fetch_sub(Active_Writer_Flag + Writer_Count_Increment - Active_Reader_Increment);
						isActive = false;
					})
			{}

			/// Default move constructor.
			WriterLockGuard(WriterLockGuard&&) = default;
		};

		// endregion

		// region ReaderLockGuard

		/// RAII reader lock guard.
		class ReaderLockGuard : public LockGuard {
		public:
			/// Creates a guard around \a value and \a notificationPolicy.
			ReaderLockGuard(std::atomic<uint32_t>& value, TReaderNotificationPolicy& notificationPolicy)
					: LockGuard([&value, &notificationPolicy]() {
						// decrease the number of readers by one
						value.fetch_sub(Active_Reader_Increment);
						notificationPolicy.readerReleased();
					})
					, m_value(value)
					, m_isWriterActive(false) {
				notificationPolicy.readerAcquired();
			}

			/// Default move constructor.
			ReaderLockGuard(ReaderLockGuard&&) = default;

		public:
			/// Promotes this reader lock to a writer lock.
			/// \note Deadlock is possible when promoteToWriter is called concurrently by multiple threads for the same lock.
			///       Each of the concurrent threads holds a reader lock, so a writer lock cannot be acquired by any thread.
			WriterLockGuard promoteToWriter() {
				markActiveWriter();

				// mark a pending write by changing the reader to a writer using CAS to guard against overflow
				YieldStepper stepper;
				uint32_t current = m_value;
				for (;;) {
					if ((current & Writer_Count_Mask) == Writer_Count_Mask)
						CATAPULT_THROW_RUNTIME_ERROR("max writer count (32767) reached");

					auto desired = current + Writer_Count_Increment - Active_Reader_Increment;
					if (m_value.compare_exchange_strong(current, desired))
						break;

					stepper.yield();
				}

				// wait for exclusive access
				AcquireWriter(m_value);
				return WriterLockGuard(m_value, m_isWriterActive);
			}

		private:
			void markActiveWriter() {
				if (m_isWriterActive)
					CATAPULT_THROW_RUNTIME_ERROR("reader lock has already been promoted");

				m_isWriterActive = true;
			}

		private:
			std::atomic<uint32_t>& m_value;
			bool m_isWriterActive;
		};

		// endregion

	public:
		/// Creates an unlocked lock.
		BasicSpinReaderWriterLock() : m_value(0)
		{}

	public:
		/// Returns \c true if there is a pending (or active) writer.
		inline bool isWriterPending() const {
			return isSet(Writer_Count_Mask);
		}

		/// Returns \c true if there is an active writer.
		inline bool isWriterActive() const {
			return isSet(Active_Writer_Flag);
		}

		/// Returns \c true if there is an active reader.
		inline bool isReaderActive() const {
			return isSet(Reader_Mask);
		}

	private:
		inline bool isSet(uint32_t mask) const {
			return 0 != (m_value & mask);
		}

	public:
		/// Blocks until a reader lock can be acquired.
		inline ReaderLockGuard acquireReader() {
			YieldStepper stepper;

			uint32_t current = m_value;
			for (;;) {
				// wait for any pending writes to complete
				if (0 != (current & Writer_Count_Mask)) {
					stepper.yield();
					current = m_value;
					continue;
				}

				// fail fast if reader count is already saturated
				if ((current & Reader_Mask) == Reader_Mask)
					CATAPULT_THROW_RUNTIME_ERROR("max reader count (65535) reached");

				// try to increment the number of readers by one
				auto desired = static_cast<uint32_t>(current + Active_Reader_Increment);
				if (m_value.compare_exchange_strong(current, desired))
					break;

				stepper.yield();
			}

			return ReaderLockGuard(m_value, *this);
		}

		/// Blocks until a writer lock can be acquired.
		inline WriterLockGuard acquireWriter() {
			// mark a pending write using CAS to guard against overflow
			YieldStepper stepper;
			uint32_t current = m_value;
			for (;;) {
				if ((current & Writer_Count_Mask) == Writer_Count_Mask)
					CATAPULT_THROW_RUNTIME_ERROR("max writer count (32767) reached");

				if (m_value.compare_exchange_strong(current, current + Writer_Count_Increment))
					break;

				stepper.yield();
			}

			// wait for exclusive access
			AcquireWriter(m_value);
			return WriterLockGuard(m_value);
		}

	private:
		static void AcquireWriter(std::atomic<uint32_t>& value) {
			YieldStepper stepper;

			// wait for exclusive access (when there is no active writer and no readers)
			uint32_t expected = value & Writer_Count_Mask;
			while (!value.compare_exchange_strong(expected, expected | Active_Writer_Flag)) {
				stepper.yield();
				expected = value & Writer_Count_Mask;
			}
		}

	private:
		std::atomic<uint32_t> m_value;
	};

	/// No-op reader notification policy.
	struct NoOpReaderNotificationPolicy {
		/// Reader was acquried by the current thread.
		constexpr void readerAcquired()
		{}

		/// Reader was released by the current thread.
		constexpr void readerReleased()
		{}
	};
}}

#ifdef ENABLE_CATAPULT_DIAGNOSTICS
#include "ReentrancyCheckReaderNotificationPolicy.h"
#endif

namespace catapult { namespace utils {

#ifdef ENABLE_CATAPULT_DIAGNOSTICS
	using DefaultReaderNotificationPolicy = ReentrancyCheckReaderNotificationPolicy;
#else
	using DefaultReaderNotificationPolicy = NoOpReaderNotificationPolicy;
#endif

	/// Default reader writer lock.
	using SpinReaderWriterLock = BasicSpinReaderWriterLock<DefaultReaderNotificationPolicy>;
}}
