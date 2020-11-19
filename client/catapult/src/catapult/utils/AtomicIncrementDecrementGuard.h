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
