#pragma once
#include "MapperInclude.h"

namespace catapult { namespace model { struct TransactionStatus; } }

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a transaction \a status to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const model::TransactionStatus& status);
}}}
