#pragma once
#include "catapult/model/RangeTypes.h"

namespace mongocxx {
	inline namespace v_noabi { class cursor; }
}

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document { class view; }
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps \a numHashes db hashes under \a cursor to a hash range.
	model::HashRange ToModel(mongocxx::cursor& cursor, size_t numHashes);

	/// Maps \a numHashes db hashes from \a hashContainer to a hash range.
	model::HashRange ToModel(const std::vector<bsoncxx::document::view>& hashContainer, size_t numHashes);
}}}
