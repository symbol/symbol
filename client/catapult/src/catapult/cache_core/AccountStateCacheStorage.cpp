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

#include "AccountStateCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/state/AccountStateAdapter.h"
#include "catapult/utils/MemoryUtils.h"
#include <vector>

namespace catapult { namespace cache {

	namespace {
		uint32_t ReadAccountInfoSize(io::InputStream& input) {
			auto accountInfoSize = io::Read32(input);
			if (accountInfoSize > model::AccountInfo_Max_Size)
				CATAPULT_THROW_RUNTIME_ERROR_1("account in state file has enormous size", accountInfoSize);

			return accountInfoSize;
		}

		void ReadAccountInfo(io::InputStream& input, uint32_t accountInfoSize, model::AccountInfo& accountInfo) {
			constexpr auto Header_Size = sizeof(accountInfoSize); // the Size field was already read

			accountInfo.Size = accountInfoSize;

			auto* pAccountInfoBytes = reinterpret_cast<uint8_t*>(&accountInfo);
			input.read({ pAccountInfoBytes + Header_Size, accountInfoSize - Header_Size });
		}
	}

	void AccountStateCacheStorage::Save(const StorageType& element, io::OutputStream& output) {
		const auto& pAccountState = element.second;
		auto pAccountInfo = state::ToAccountInfo(*pAccountState);
		output.write({ reinterpret_cast<const uint8_t*>(pAccountInfo.get()), pAccountInfo->Size });
	}

	std::unique_ptr<model::AccountInfo> AccountStateCacheStorage::Load(io::InputStream& input) {
		auto accountInfoSize = ReadAccountInfoSize(input);
		auto pAccountInfo = utils::MakeUniqueWithSize<model::AccountInfo>(accountInfoSize);
		ReadAccountInfo(input, accountInfoSize, *pAccountInfo);
		return pAccountInfo;
	}

	void AccountStateCacheStorage::LoadInto(io::InputStream& input, DestinationType& cacheDelta, LoadStateType& state) {
		auto accountInfoSize = ReadAccountInfoSize(input);
		state.resize(accountInfoSize);

		auto& accountInfo = reinterpret_cast<model::AccountInfo&>(*state.data());
		ReadAccountInfo(input, accountInfoSize, accountInfo);
		cacheDelta.addAccount(accountInfo);
	}
}}
