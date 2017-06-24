#pragma once
#include <atomic>

namespace catapult { namespace utils {

	/// RAII class that increments an atomic on construction and decrements it on destruction.
	template<typename T>
	class AtomicIncrementDecrementGuard {
	public:
		explicit AtomicIncrementDecrementGuard(std::atomic<T>& value) : m_value(value) {
			++m_value;
		}

		~AtomicIncrementDecrementGuard() {
			--m_value;
		}

	private:
		std::atomic<T>& m_value;

	public:
		AtomicIncrementDecrementGuard<T>(const AtomicIncrementDecrementGuard<T>& rhs) = default;
		AtomicIncrementDecrementGuard<T>& operator=(const AtomicIncrementDecrementGuard<T>& rhs) = default;
	};

	/// Factory function for creating AtomicIncrementDecrementGuard<T>.
	template<typename T>
	AtomicIncrementDecrementGuard<T> MakeIncrementDecrementGuard(std::atomic<T>& value) {
		return AtomicIncrementDecrementGuard<T>(value);
	}
}}
