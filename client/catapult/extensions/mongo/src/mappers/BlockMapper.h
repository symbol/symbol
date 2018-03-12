#pragma once
#include "MapperInclude.h"
#include "catapult/state/BlockDifficultyInfo.h"

namespace catapult { namespace model { struct BlockElement; } }

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a \a blockElement to the corresponding db model entity.
	bsoncxx::document::value ToDbModel(const model::BlockElement& blockElement);

	/// Maps \a document to a block difficulty info.
	state::BlockDifficultyInfo ToDifficultyInfo(const bsoncxx::document::view& document);
}}}
