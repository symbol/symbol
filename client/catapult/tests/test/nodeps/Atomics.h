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
#include "Waits.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"

namespace catapult { namespace test {

	/// Boolean flag that is automatically set on destruction.
	class AutoSetFlag : public utils::MoveOnly {
	public:
		/// State of an auto set boolean flag.
		class State {
		public:
			/// Creates a flag.
			State() : m_flag(false), m_numWaiters(0)
			{}

		public:
			/// Sets the flag.
			void set() {
				if (m_flag)
					return;

				CATAPULT_LOG(debug) << "setting auto set flag";
				m_flag = true;
			}

			/// Gets a value indicating whether the flag is set or not.
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

	public:
		/// Creates a flag.
		AutoSetFlag() : m_pState(std::make_shared<State>())
		{}

		/// Destroys the flag.
		~AutoSetFlag() {
			m_pState->set(); // unblock any waiting threads
		}

	public:
		/// Gets the underlying state.
		std::shared_ptr<State> state() const {
			return m_pState;
		}

	private:
		std::shared_ptr<State> m_pState;
	};
}}
