#pragma once
#include "ChainScoreProvider.h"
#include "ExternalCacheStorage.h"
#include "catapult/consumers/StateChangeInfo.h"
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

		void notifyStateChange(const consumers::StateChangeInfo& stateChangeInfo) override {
			m_pCacheStorage->saveDelta(stateChangeInfo.CacheDelta);
		}

	private:
		std::unique_ptr<ChainScoreProvider> m_pChainScoreProvider;
		std::unique_ptr<ExternalCacheStorage> m_pCacheStorage;
	};
}}
