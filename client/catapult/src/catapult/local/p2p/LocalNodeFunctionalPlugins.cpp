#include "LocalNodeFunctionalPlugins.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/FutureUtils.h"

namespace catapult { namespace local { namespace p2p {

	RemoteChainHeightsRetriever CreateRemoteChainHeightsRetriever(net::PacketIoPicker& packetIoPicker) {
		return [&packetIoPicker](size_t numPeers) -> thread::future<std::vector<Height>> {
			std::vector<thread::future<Height>> heightFutures;
			auto timeout = utils::TimeSpan::FromSeconds(5);

			auto packetIoPairs = net::PickMultiple(packetIoPicker, numPeers, timeout);
			if (packetIoPairs.empty()) {
				CATAPULT_LOG(warning) << "could not find any peer for detecting chain heights";
				return thread::make_ready_future(std::vector<Height>());
			}

			for (const auto& packetIoPair : packetIoPairs) {
				auto pChainApi = api::CreateRemoteChainApi(packetIoPair.io());
				heightFutures.push_back(pChainApi->chainInfo().then([](auto&& infoFuture) { return infoFuture.get().Height; }));
			}

			return thread::when_all(std::move(heightFutures)).then([](auto&& completedFutures) {
				return thread::get_all_ignore_exceptional(completedFutures.get());
			});
		};
	}

	ChainSyncedPredicate CreateChainSyncedPredicate(
			const io::BlockStorageCache& storage,
			const NetworkChainHeightSupplier& networkChainHeightSupplier) {
		// if the local node's chain is too far behind, some operations like harvesting or
		// processing of pushed elements should not be allowed
		// for a public network this should always return true since an evil node could supply a fake chain height
		return [&storage, networkChainHeightSupplier]() -> bool {
			auto storageView = storage.view();
			return networkChainHeightSupplier() < storageView.chainHeight().unwrap() + 4;
		};
	}
}}}
