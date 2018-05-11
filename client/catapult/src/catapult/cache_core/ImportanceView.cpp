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
