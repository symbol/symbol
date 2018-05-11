/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace mocks {

	/// Mock noop state change subscriber implementation.
	class MockStateChangeSubscriber : public subscribers::StateChangeSubscriber {
	public:
		/// Creates a subscriber.
		MockStateChangeSubscriber()
				: m_numScoreChanges(0)
				, m_numStateChanges(0)
		{}

	public:
		/// Gets the number of score changes.
		size_t numScoreChanges() const {
			return m_numScoreChanges;
		}

		/// Gets the number of state changes.
		size_t numStateChanges() const {
			return m_numStateChanges;
		}

		/// Gets the last chain score.
		model::ChainScore lastChainScore() const {
			return m_lastChainScore;
		}

	public:
		void notifyScoreChange(const model::ChainScore& score) override {
			m_lastChainScore = score;
			++m_numScoreChanges;
		}

		void notifyStateChange(const consumers::StateChangeInfo&) override {
			++m_numStateChanges;
		}

	private:
		size_t m_numScoreChanges;
		size_t m_numStateChanges;
		model::ChainScore m_lastChainScore;
	};
}}
