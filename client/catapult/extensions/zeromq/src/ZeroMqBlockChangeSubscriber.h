#pragma once
#include "catapult/io/BlockChangeSubscriber.h"
#include <memory>

namespace catapult { namespace zeromq { class ZeroMqEntityPublisher; } }

namespace catapult { namespace zeromq {

	/// Creates a zeromq block change subscriber around an entity \a publisher.
	std::unique_ptr<io::BlockChangeSubscriber> CreateZeroMqBlockChangeSubscriber(ZeroMqEntityPublisher& publisher);
}}
