#pragma once
#include "catapult/cache/UtChangeSubscriber.h"
#include <memory>

namespace catapult { namespace zeromq { class ZeroMqEntityPublisher; } }

namespace catapult { namespace zeromq {

	/// Creates a zeromq unconfirmed transactions subscriber around an entity \a publisher.
	std::unique_ptr<cache::UtChangeSubscriber> CreateZeroMqUtChangeSubscriber(ZeroMqEntityPublisher& publisher);
}}
