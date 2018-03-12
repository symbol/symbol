#pragma once
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include <memory>

namespace catapult { namespace zeromq { class ZeroMqEntityPublisher; } }

namespace catapult { namespace zeromq {

	/// Creates a zeromq transaction status subscriber around an entity \a publisher.
	std::unique_ptr<subscribers::TransactionStatusSubscriber> CreateZeroMqTransactionStatusSubscriber(ZeroMqEntityPublisher& publisher);
}}
