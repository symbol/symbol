#include "ImportanceView.h"
#include "AccountStateCache.h"
#include "catapult/model/Address.h"

namespace catapult { namespace cache {

	namespace {
		std::shared_ptr<const state::AccountState> FindStateWithImportance(
				const ReadOnlyAccountStateCache& cache,
				const Key& publicKey,
				Height height) {
			auto pState = cache.findAccount(publicKey);

			// if state could not be accessed by public key, try searching by address
			if (!pState)
				pState = cache.findAccount(model::PublicKeyToAddress(publicKey, cache.networkIdentifier()));

			if (!pState || model::ConvertToImportanceHeight(height, cache.importanceGrouping()) != pState->ImportanceInfo.height())
				return nullptr;

			return pState;
		}
	}

	bool ImportanceView::tryGetAccountImportance(
			const Key& publicKey,
			Height height,
			Importance& importance) const {
		auto pState = FindStateWithImportance(m_cache, publicKey, height);

		if (!pState)
			return false;

		importance = pState->ImportanceInfo.current();
		return true;
	}

	Importance ImportanceView::getAccountImportanceOrDefault(const Key& publicKey, Height height) const {
		Importance importance;
		return tryGetAccountImportance(publicKey, height, importance) ? importance : Importance(0);
	}

	bool ImportanceView::canHarvest(const Key& publicKey, Height height, Amount minHarvestingBalance) const {
		auto pState = FindStateWithImportance(m_cache, publicKey, height);

		return pState
				&& pState->ImportanceInfo.current() > Importance(0)
				&& pState->Balances.get(Xem_Id) >= minHarvestingBalance;
	}
}}
