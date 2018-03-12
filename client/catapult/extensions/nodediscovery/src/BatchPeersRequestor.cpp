#include "BatchPeersRequestor.h"
#include "nodediscovery/src/api/RemoteNodeApi.h"
#include "catapult/chain/NodeInteractionResult.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/ThrottleLogger.h"

namespace catapult { namespace nodediscovery {

	BatchPeersRequestor::BatchPeersRequestor(const net::PacketIoPickerContainer& packetIoPickers, const NodesConsumer& nodesConsumer)
			: m_packetIoPickers(packetIoPickers)
			, m_nodesConsumer(nodesConsumer)
	{}

	thread::future<bool> BatchPeersRequestor::findPeersOfPeers(const utils::TimeSpan& timeout) const {
		auto packetIoPairs = m_packetIoPickers.pickMatching(timeout, ionet::NodeRoles::None);
		if (packetIoPairs.empty()) {
			CATAPULT_LOG_THROTTLE(warning, 60'000) << "no packet io available for requesting peers";
			return thread::make_ready_future(false);
		}

		auto i = 0u;
		std::vector<thread::future<chain::NodeInteractionResult>> futures(packetIoPairs.size());
		for (const auto& packetIoPair : packetIoPairs) {
			auto peersInfoFuture = api::CreateRemoteNodeApi(*packetIoPair.io())->peersInfo();
			futures[i++] = peersInfoFuture.then([nodesConsumer = m_nodesConsumer, packetIoPair](auto&& nodesFuture) {
				try {
					auto nodes = nodesFuture.get();
					CATAPULT_LOG(debug) << "partner node " << packetIoPair.node() << " returned " << nodes.size() << " peers";
					nodesConsumer(nodes);
					return chain::NodeInteractionResult::Success;
				} catch (const catapult_runtime_error& e) {
					CATAPULT_LOG(warning) << "exception thrown while requesting peers: " << e.what();
					return chain::NodeInteractionResult::Failure;
				}
			});
		}

		return thread::when_all(std::move(futures)).then([](auto&&) {
			return true;
		});
	}
}}
