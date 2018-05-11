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

#include "AccountStateAdapter.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace state {

	AccountState ToAccountState(const model::AccountInfo& info) {
		AccountState accountState(info.Address, info.AddressHeight);
		accountState.PublicKey = info.PublicKey;
		accountState.PublicKeyHeight = info.PublicKeyHeight;

		for (auto i = Importance_History_Size; i > 0; --i) {
			auto importanceHeight = info.ImportanceHeights[i - 1];
			if (model::ImportanceHeight() == importanceHeight)
				continue;

			accountState.ImportanceInfo.set(info.Importances[i - 1], importanceHeight);
		}

		auto pMosaic = info.MosaicsPtr();
		for (auto i = 0u; i < info.MosaicsCount; ++i, ++pMosaic)
			accountState.Balances.credit(pMosaic->MosaicId, pMosaic->Amount);

		return accountState;
	}

	std::unique_ptr<model::AccountInfo> ToAccountInfo(const AccountState& accountState) {
		auto numMosaics = utils::checked_cast<size_t, uint16_t>(accountState.Balances.size());
		uint32_t entitySize = sizeof(model::AccountInfo) + numMosaics * sizeof(model::Mosaic);
		auto pAccountInfo = utils::MakeUniqueWithSize<model::AccountInfo>(entitySize);
		pAccountInfo->Size = entitySize;
		pAccountInfo->Address = accountState.Address;
		pAccountInfo->AddressHeight = accountState.AddressHeight;
		pAccountInfo->PublicKey = accountState.PublicKey;
		pAccountInfo->PublicKeyHeight = accountState.PublicKeyHeight;

		auto i = 0u;
		for (const auto& pair : accountState.ImportanceInfo) {
			pAccountInfo->Importances[i] = pair.Importance;
			pAccountInfo->ImportanceHeights[i] = pair.Height;
			++i;
		}

		pAccountInfo->MosaicsCount = numMosaics;
		auto pMosaic = pAccountInfo->MosaicsPtr();
		for (const auto& pair : accountState.Balances) {
			pMosaic->MosaicId = pair.first;
			pMosaic->Amount = pair.second;
			++pMosaic;
		}

		return pAccountInfo;
	}
}}
