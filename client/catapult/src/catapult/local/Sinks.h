#pragma once
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/InputUtils.h"

namespace catapult { namespace local {

	/// A new block sink prototype;
	using NewBlockSink = consumers::NewBlockSink;

	/// A new transactions sink prototype that does not take ownership of new infos.
	using SharedNewTransactionsSink = std::function<void (const consumers::TransactionInfos&)>;
}}
