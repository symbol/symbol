#pragma once
#include "catapult/model/Elements.h"
#include "catapult/functions.h"
#include <vector>

namespace catapult { namespace ionet { struct Packet; } }

namespace catapult { namespace parsers {

	/// Tries to parse transaction elements out of \a packet and into \a elements with a validity check (\a isValid).
	bool TryParseTransactionElements(
			const ionet::Packet& packet,
			const predicate<const model::Transaction&>& isValid,
			std::vector<model::TransactionElement>& elements);
}}
