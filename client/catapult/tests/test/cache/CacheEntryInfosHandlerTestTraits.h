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
#include "catapult/ionet/PacketPayload.h"
#include "catapult/model/CacheEntryInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Traits for diagnostic handlers using CacheEntryInfo.
	template<typename TIdentifier, ionet::PacketType PacketType>
	struct CacheEntryInfoHandlerTestTraits {
	public:
		using Infos = std::vector<std::shared_ptr<const model::CacheEntryInfo<TIdentifier>>>;
		using RequestStructureType = TIdentifier;
		using ResponseType = Infos;
		static constexpr auto Packet_Type = PacketType;
		static constexpr auto Message = "info at ";

	public:
		struct ResponseState {};

	public:
		static ResponseType CreateResponse(size_t count, const ResponseState&) {
			ResponseType infos;
			for (auto i = 0u; i < count; ++i) {
				auto pInfo = std::make_shared<model::CacheEntryInfo<TIdentifier>>();
				pInfo->Id = ToIdentifier(123 + i);
				pInfo->DataSize = 0;
				pInfo->Size = static_cast<uint32_t>(model::CacheEntryInfo<TIdentifier>::CalculateRealSize(*pInfo));
				infos.push_back(pInfo);
			}

			return infos;
		}

		static size_t TotalSize(const ResponseType& result) {
			return result.size() * sizeof(model::CacheEntryInfo<TIdentifier>);
		}

		static void AssertExpectedResponse(const ionet::PacketPayload& payload, const ResponseType& expectedResult) {
			ASSERT_EQ(expectedResult.size(), payload.buffers().size());

			auto i = 0u;
			for (const auto& pExpectedInfo : expectedResult) {
				const auto& info = reinterpret_cast<const model::CacheEntryInfo<TIdentifier>&>(*payload.buffers()[i].pData);
				EXPECT_EQ(pExpectedInfo->Id, info.Id) << Message << i;
				++i;
			}
		}

	private:
		template<typename X>
		static TIdentifier ToIdentifier(uint32_t value) {
			if constexpr (utils::traits::is_pod_v<TIdentifier>)
				return TIdentifier(value)
			else
				return TIdentifier{ { static_cast<uint8_t>(value) } };
		}
	};
}}
