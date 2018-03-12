#include "RemoteNodeApi.h"
#include "nodediscovery/src/NodePingUtils.h"
#include "catapult/api/RemoteRequestDispatcher.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct NodeInfoTraits {
		public:
			using ResultType = ionet::Node;
			static constexpr auto PacketType() { return ionet::PacketType::Node_Discovery_Pull_Ping; }
			static constexpr auto FriendlyName() { return "node info"; }

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(PacketType());
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				return nodediscovery::TryParseNodePacket(packet, result);
			}
		};

		struct PeersInfoTraits {
		public:
			using ResultType = ionet::NodeSet;
			static constexpr auto PacketType() { return ionet::PacketType::Node_Discovery_Pull_Peers; }
			static constexpr auto FriendlyName() { return "peers info"; }

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(PacketType());
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
