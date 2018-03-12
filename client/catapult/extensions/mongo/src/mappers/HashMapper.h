#pragma once
#include "MapperInclude.h"
#include "catapult/model/RangeTypes.h"

namespace catapult { namespace mongo { namespace mappers {

	/// Maps \a numHashes db hashes under \a cursor to a hash range.
	model::HashRange ToModel(mongocxx::cursor& cursor, size_t numHashes);

	/// Maps \a numHashes db hashes from \a hashContainer to a hash range.
	model::HashRange ToModel(const std::vector<bsoncxx::document::view>& hashContainer, size_t numHashes);
}}}
