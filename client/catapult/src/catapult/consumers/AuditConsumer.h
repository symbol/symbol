#pragma once
#include "catapult/disruptor/DisruptorConsumer.h"

namespace catapult { namespace consumers {

	/// Creates an audit consumer that saves all consumer inputs to \a auditDirectory.
	disruptor::ConstDisruptorConsumer CreateAuditConsumer(const std::string& auditDirectory);
}}
