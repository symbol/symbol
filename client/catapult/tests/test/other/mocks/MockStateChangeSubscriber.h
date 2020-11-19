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
#include "catapult/subscribers/StateChangeInfo.h"
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace mocks {

	/// Mock state change subscriber implementation.
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
		const model::ChainScore& lastChainScore() const {
			return m_lastChainScore;
		}

		/// Gets the last state change info.
		const subscribers::StateChangeInfo& lastStateChangeInfo() const {
			return *m_pLastStateChangeInfo;
		}

	public:
		/// Sets the \a consumer that will be called with cache changes.
		void setCacheChangesConsumer(const consumer<const cache::CacheChanges&>& consumer) {
			m_cacheChangesConsumer = consumer;
		}

	public:
		void notifyScoreChange(const model::ChainScore& score) override {
			m_lastChainScore = score;
			++m_numScoreChanges;
		}

		void notifyStateChange(const subscribers::StateChangeInfo& stateChangeInfo) override {
			++m_numStateChanges;

			if (m_cacheChangesConsumer)
				m_cacheChangesConsumer(stateChangeInfo.CacheChanges);

			// for test purposes, ignore const and take ownership of stateChangeInfo.CacheChanges
			m_pLastStateChangeInfo = std::make_unique<subscribers::StateChangeInfo>(
					std::move(const_cast<cache::CacheChanges&>(stateChangeInfo.CacheChanges)),
					stateChangeInfo.ScoreDelta,
					stateChangeInfo.Height);
		}

	private:
		size_t m_numScoreChanges;
		size_t m_numStateChanges;
		model::ChainScore m_lastChainScore;
		std::unique_ptr<subscribers::StateChangeInfo> m_pLastStateChangeInfo;
		consumer<const cache::CacheChanges&> m_cacheChangesConsumer;
	};
}}
