#pragma once
#include "catapult/thread/Future.h"
#include "catapult/types.h"
#include <atomic>
#include <functional>
#include <vector>

namespace catapult { namespace local { namespace p2p {

	/// An atomic network chain height.
	using NetworkChainHeight = std::atomic<Height::ValueType>;

	/// A supplier that returns the chain height of the network seen by the local node.
	using NetworkChainHeightSupplier = std::function<Height::ValueType ()>;

	/// A retriever that returns the network chain heights for a number of peers.
	using RemoteChainHeightsRetriever = std::function<thread::future<std::vector<Height>> (size_t)>;
}}}
