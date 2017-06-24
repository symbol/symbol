#pragma once
#include "plugins/txes/multisig/src/state/MultisigEntry.h"

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document {
			class value;
			class view;
		}
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a multisig \a entry to the corresponding db model entity.
	bsoncxx::document::value ToDbModel(const state::MultisigEntry& entry);

	/// Maps a database \a document to the corresponding model entity.
	state::MultisigEntry ToMultisigEntry(const bsoncxx::document::view& document);
}}}
