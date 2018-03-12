#pragma once
#include "catapult/api/ChainApi.h"
#include "catapult/thread/Future.h"

namespace catapult {
	namespace ionet { class Node; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace tools { namespace health {

	/// Creates a future for retrieving the chain info of the specified api \a node over REST API using \a pool.
	/// \note Default REST API port is assumed.
	thread::future<api::ChainInfo> CreateApiNodeChainInfoFuture(thread::IoServiceThreadPool& pool, const ionet::Node& node);
}}}
