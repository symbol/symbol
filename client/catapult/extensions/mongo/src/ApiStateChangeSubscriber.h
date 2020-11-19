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
#include "ChainScoreProvider.h"
#include "ExternalCacheStorage.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace mongo {

	/// Api state change subscriber.
	class ApiStateChangeSubscriber : public subscribers::StateChangeSubscriber {
	public:
		/// Creates a subscriber around \a pChainScoreProvider and \a pCacheStorage.
		ApiStateChangeSubscriber(
				std::unique_ptr<ChainScoreProvider>&& pChainScoreProvider,
				std::unique_ptr<ExternalCacheStorage>&& pCacheStorage)
				: m_pChainScoreProvider(std::move(pChainScoreProvider))
				, m_pCacheStorage(std::move(pCacheStorage))
		{}

	public:
		void notifyScoreChange(const model::ChainScore& chainScore) override {
			m_pChainScoreProvider->saveScore(chainScore);
		}

		void notifyStateChange(const subscribers::StateChangeInfo& stateChangeInfo) override {
			m_pCacheStorage->saveDelta(stateChangeInfo.CacheChanges);
		}

	private:
		std::unique_ptr<ChainScoreProvider> m_pChainScoreProvider;
		std::unique_ptr<ExternalCacheStorage> m_pCacheStorage;
	};
}}
