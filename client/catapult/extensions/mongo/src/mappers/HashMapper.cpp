/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
					CATAPULT_THROW_RUNTIME_ERROR("db inconsistent, more hashes in db than expected");

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
