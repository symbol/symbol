#include "HashMapper.h"
#include "MapperUtils.h"

namespace catapult { namespace mongo { namespace mappers {
	namespace {
		template<typename TIterable>
		model::HashRange DbHashesToHashRange(TIterable& iterable, size_t numHashes) {
			auto hashRange = model::HashRange::PrepareFixed(numHashes);
			auto hashRangeIter = hashRange.begin();

			size_t boundCheck = 0;
			for (const auto& dbHash : iterable) {
				if (numHashes == boundCheck++)
					CATAPULT_THROW_RUNTIME_ERROR("db inconsistent, more hashes in db then expected");

				DbBinaryToModelArray(*hashRangeIter++, dbHash["meta"]["hash"].get_binary());
			}

			if (numHashes != boundCheck)
				CATAPULT_THROW_RUNTIME_ERROR("db inconsistent, not enough hashes");

			return hashRange;
		}
	}

	model::HashRange ToModel(mongocxx::cursor& cursor, size_t numHashes) {
		return DbHashesToHashRange(cursor, numHashes);
	}

	model::HashRange ToModel(const std::vector<bsoncxx::document::view>& hashContainer, size_t numHashes) {
		return DbHashesToHashRange(hashContainer, numHashes);
	}
}}}
