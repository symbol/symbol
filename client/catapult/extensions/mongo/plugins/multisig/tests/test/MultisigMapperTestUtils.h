#pragma once
#include "plugins/txes/multisig/src/state/MultisigEntry.h"
#include <bsoncxx/json.hpp>

namespace catapult { namespace test {

	/// Verifies that db multisig (\a dbMultisig) is equivalent to model multisig \a entry and \a address.
	void AssertEqualMultisigData(const state::MultisigEntry& entry, const Address& address, const bsoncxx::document::view& dbMultisig);
}}
