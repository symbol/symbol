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

#include "ToolConversionUtils.h"
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/state/AccountStateSerializer.h"

namespace catapult { namespace tools {

	namespace {
		bool IsValidInfo(const model::CacheEntryInfo<Address>& cacheEntryInfo) {
			return cacheEntryInfo.HasData() && !cacheEntryInfo.IsTooLarge();
		}
	}

	std::vector<state::AccountState> ParseAccountStates(const model::EntityRange<model::CacheEntryInfo<Address>>& cacheEntryInfos) {
		if (cacheEntryInfos.empty())
			return std::vector<state::AccountState>();

		std::vector<state::AccountState> accountStates;
		for (const auto& cacheEntryInfo : cacheEntryInfos) {
			if (!IsValidInfo(cacheEntryInfo))
				continue;

			RawBuffer buffer{ cacheEntryInfo.DataPtr(), cacheEntryInfo.DataSize };
			io::BufferInputStreamAdapter<RawBuffer> inputStream(buffer);
			accountStates.push_back(state::AccountStateNonHistoricalSerializer::Load(inputStream));
		}

		return accountStates;
	}
}}
