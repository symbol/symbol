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

#include "RemoteNodeApi.h"
#include "nodediscovery/src/NodePingUtils.h"
#include "catapult/api/RemoteRequestDispatcher.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct NodeInfoTraits {
		public:
			using ResultType = ionet::Node;
			static constexpr auto Packet_Type = ionet::PacketType::Node_Discovery_Pull_Ping;
			static constexpr auto Friendly_Name = "node info";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				return nodediscovery::TryParseNodePacket(packet, result);
			}
		};

		struct PeersInfoTraits {
		public:
			using ResultType = ionet::NodeSet;
			static constexpr auto Packet_Type = ionet::PacketType::Node_Discovery_Pull_Peers;
			static constexpr auto Friendly_Name = "peers info";

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(Packet_Type);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				return nodediscovery::TryParseNodesPacket(packet, result);
			}
		};

		// endregion

		class DefaultRemoteNodeApi : public RemoteNodeApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			explicit DefaultRemoteNodeApi(ionet::PacketIo& io) : m_impl(io)
			{}

		public:
			FutureType<NodeInfoTraits> nodeInfo() const override {
				return m_impl.dispatch(NodeInfoTraits());
			}

			FutureType<PeersInfoTraits> peersInfo() const override {
				return m_impl.dispatch(PeersInfoTraits());
			}

		private:
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<RemoteNodeApi> CreateRemoteNodeApi(ionet::PacketIo& io) {
		return std::make_unique<DefaultRemoteNodeApi>(io);
	}
}}
