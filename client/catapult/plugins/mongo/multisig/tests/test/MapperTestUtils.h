#pragma once

namespace bsoncxx { inline namespace v_noabi { namespace document { class view; } } }

namespace catapult { namespace state { class MultisigEntry; } }

namespace catapult { namespace mongo { namespace test {

	/// Verifies that db multisig (\a dbMultisig) and model multisig \a entry are equivalent.
	void AssertEqualMultisigData(const state::MultisigEntry& entry, const bsoncxx::document::view& dbMultisig);
}}}
