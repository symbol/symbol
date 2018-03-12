#pragma once
#include "catapult/model/CosignedTransactionInfo.h"
#include "catapult/functions.h"
#include <vector>

namespace catapult { namespace ionet { struct Packet; } }

namespace catapult { namespace api {

	/// Extracts cosigned transaction infos from \a packet with a validity check (\a isValid).
	/// \note If the packet is invalid and/or contains partial infos, the returned vector will be empty.
	std::vector<model::CosignedTransactionInfo> ExtractCosignedTransactionInfosFromPacket(
			const ionet::Packet& packet,
			const predicate<const model::Transaction&>& isValid);
}}
