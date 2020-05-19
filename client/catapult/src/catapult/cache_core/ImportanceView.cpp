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
		template<typename TAction>
		bool ForwardIfAccountHasImportanceAtHeight(
				const state::AccountState& accountState,
				const ReadOnlyAccountStateCache& cache,
				Height height,
				TAction action) {
			if (state::AccountType::Remote == accountState.AccountType) {
				auto linkedAccountStateIter = cache.find(state::GetLinkedPublicKey(accountState));
				const auto& linkedAccountState = linkedAccountStateIter.get();

				// this check is merely a precaution and will only fire if there is a bug that has corrupted links
				RequireLinkedRemoteAndMainAccounts(accountState, linkedAccountState);

				return ForwardIfAccountHasImportanceAtHeight(linkedAccountState, cache, height, action);
			}

			auto importanceHeight = model::ConvertToImportanceHeight(height, cache.importanceGrouping());
			if (importanceHeight != accountState.ImportanceSnapshots.height())
				return false;

			return action(accountState);
		}

		template<typename TAction>
		bool FindAccountStateWithImportance(
				const ReadOnlyAccountStateCache& cache,
				const Address& address,
				Height height,
				TAction action) {
			auto accountStateAddressIter = cache.find(address);
			if (accountStateAddressIter.tryGet())
				return ForwardIfAccountHasImportanceAtHeight(accountStateAddressIter.get(), cache, height, action);

			return false;
		}

		template<typename TAction>
		bool FindAccountStateWithImportance(const ReadOnlyAccountStateCache& cache, const Key& publicKey, Height height, TAction action) {
			auto accountStateKeyIter = cache.find(publicKey);
			if (accountStateKeyIter.tryGet())
				return ForwardIfAccountHasImportanceAtHeight(accountStateKeyIter.get(), cache, height, action);

			// if state could not be accessed by public key, try searching by address
			auto address = model::PublicKeyToAddress(publicKey, cache.networkIdentifier());
			return FindAccountStateWithImportance(cache, address, height, action);
		}
	}

	ImportanceView::ImportanceView(const ReadOnlyAccountStateCache& cache) : m_cache(cache)
	{}

	bool ImportanceView::tryGetAccountImportance(const Key& publicKey, Height height, Importance& importance) const {
		return FindAccountStateWithImportance(m_cache, publicKey, height, [&importance](const auto& accountState) {
			importance = accountState.ImportanceSnapshots.current();
			return true;
		});
	}

	Importance ImportanceView::getAccountImportanceOrDefault(const Key& publicKey, Height height) const {
		Importance importance;
		return tryGetAccountImportance(publicKey, height, importance) ? importance : Importance(0);
	}

	bool ImportanceView::canHarvest(const Address& address, Height height) const {
		auto mosaicId = m_cache.harvestingMosaicId();
		auto minHarvesterBalance = m_cache.minHarvesterBalance();
		auto maxHarvesterBalance = m_cache.maxHarvesterBalance();
		return FindAccountStateWithImportance(m_cache, address, height, [mosaicId, minHarvesterBalance, maxHarvesterBalance](
				const auto& accountState) {
			auto currentImportance = accountState.ImportanceSnapshots.current();
			if (Importance(0) == currentImportance)
				return false;

			auto balance = accountState.Balances.get(mosaicId);
			return minHarvesterBalance <= balance && balance <= maxHarvesterBalance;
		});
	}
}}
