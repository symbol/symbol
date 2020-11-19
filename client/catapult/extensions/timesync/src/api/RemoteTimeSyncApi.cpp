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

#include "RemoteTimeSyncApi.h"
#include "TimeSyncPackets.h"
#include "catapult/api/RemoteRequestDispatcher.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct NetworkTimeTraits {
		public:
			using ResultType = timesync::CommunicationTimestamps;
			static constexpr auto Packet_Type = ionet::PacketType::Time_Sync_Network_Time;
			static constexpr auto Friendly_Name = "network time";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				const auto* pResponse = ionet::CoercePacket<api::NetworkTimePacket>(&packet);
				if (!pResponse)
					return false;

				result = pResponse->CommunicationTimestamps;
				return true;
			}
		};

		// endregion

		class DefaultRemoteTimeSyncApi : public RemoteTimeSyncApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			explicit DefaultRemoteTimeSyncApi(ionet::PacketIo& io) : m_impl(io)
			{}

		public:
			FutureType<NetworkTimeTraits> networkTime() const override {
				return m_impl.dispatch(NetworkTimeTraits());
			}

		private:
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteTimeSyncApi> CreateRemoteTimeSyncApi(ionet::PacketIo& io) {
		return std::make_unique<DefaultRemoteTimeSyncApi>(io);
	}
}}
