#pragma once
#include "ZeroMqEntityPublisher.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace zeromq {

	/// Creates a topic around \a marker and \a address
	std::vector<uint8_t> CreateTopic(TransactionMarker marker, const Address& address);
}}
