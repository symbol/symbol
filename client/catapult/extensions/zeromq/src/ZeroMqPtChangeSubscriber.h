#pragma once
#include "catapult/cache/PtChangeSubscriber.h"
#include <memory>

namespace catapult { namespace zeromq { class ZeroMqEntityPublisher; } }

namespace catapult { namespace zeromq {

	/// Creates a zeromq partial transactions subscriber around an entity \a publisher.
	std::unique_ptr<cache::PtChangeSubscriber> CreateZeroMqPtChangeSubscriber(ZeroMqEntityPublisher& publisher);
}}
