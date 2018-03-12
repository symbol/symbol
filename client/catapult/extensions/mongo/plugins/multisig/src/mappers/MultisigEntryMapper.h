#pragma once
#include "mongo/src/mappers/MapperInclude.h"
#include "plugins/txes/multisig/src/state/MultisigEntry.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Maps a multisig \a entry and \a accountAddress to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const state::MultisigEntry& entry, const Address& accountAddress);

	/// Maps a database \a document to the corresponding model value.
	state::MultisigEntry ToMultisigEntry(const bsoncxx::document::view& document);
}}}
