#include "ImportanceView.h"
#include "AccountStateCache.h"
#include "catapult/model/Address.h"

namespace catapult { namespace cache {

	namespace {
		const state::AccountState* FindStateWithImportance(const ReadOnlyAccountStateCache& cache, const Key& publicKey, Height height) {
			auto pAccountState = cache.tryGet(publicKey);

			// if state could not be accessed by public key, try searching by address
			if (!pAccountState)
				pAccountState = cache.tryGet(model::PublicKeyToAddress(publicKey, cache.networkIdentifier()));

			auto importanceHeight = model::ConvertToImportanceHeight(height, cache.importanceGrouping());
			if (!pAccountState || importanceHeight != pAccountState->ImportanceInfo.height())
				return nullptr;

			return pAccountState;
		}
	}

	bool ImportanceView::tryGetAccountImportance(const Key& publicKey, Height height, Importance& importance) const {
		auto pAccountState = FindStateWithImportance(m_cache, publicKey, height);

		if (!pAccountState)
			return false;

		importance = pAccountState->ImportanceInfo.current();
		return true;
	}

	Importance ImportanceView::getAccountImportanceOrDefault(const Key& publicKey, Height height) const {
		Importance importance;
		return tryGetAccountImportance(publicKey, height, importance) ? importance : Importance(0);
	}

	bool ImportanceView::canHarvest(const Key& publicKey, Height height, Amount minHarvestingBalance) const {
		auto pAccountState = FindStateWithImportance(m_cache, publicKey, height);

		return pAccountState
				&& pAccountState->ImportanceInfo.current() > Importance(0)
				&& pAccountState->Balances.get(Xem_Id) >= minHarvestingBalance;
	}
}}
