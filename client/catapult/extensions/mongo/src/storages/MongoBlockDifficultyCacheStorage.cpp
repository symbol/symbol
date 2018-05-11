/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "MongoBlockDifficultyCacheStorage.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoDatabase.h"
#include "mongo/src/mappers/BlockMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/cache_core/BlockDifficultyCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		void LoadDifficulties(
				const mongocxx::database& database,
				Height height,
				size_t numDifficulties,
				cache::BlockDifficultyCacheDelta& cacheDelta) {
			auto blocks = database["blocks"];
			auto filter = document()
					<< "block.height"
					<< open_document
						<< "$gte" << static_cast<int64_t>(height.unwrap())
						<< "$lt" << static_cast<int64_t>(height.unwrap() + numDifficulties)
					<< close_document
					<< finalize;

			mongocxx::options::find options;
			options.sort(document() << "block.height" << 1 << finalize);
			options.projection(document()
					<< "block.height" << 1
					<< "block.timestamp" << 1
					<< "block.difficulty" << 1
					<< finalize);

			auto cursor = blocks.find(filter.view(), options);
			for (const auto& element : cursor)
				cacheDelta.insert(mappers::ToDifficultyInfo(element["block"].get_document()));
		}

		class MongoBlockDifficultyCacheStorage : public ExternalCacheStorageT<cache::BlockDifficultyCache> {
		public:
			explicit MongoBlockDifficultyCacheStorage(MongoDatabase&& database, uint64_t difficultyHistorySize)
					: m_database(std::move(database))
					, m_difficultyHistorySize(difficultyHistorySize)
			{}

		private:
			void saveDelta(const cache::BlockDifficultyCacheDelta&) {
				// no action needed since block difficulties are saved in the blocks collection
			}

		private:
			void loadAll(cache::BlockDifficultyCacheDelta& cache, Height chainHeight, const LoadCheckpointFunc&) const {
				auto currentHeight = chainHeight.unwrap();
				if (0u == currentHeight)
					return;

				auto numDifficulties = std::min(m_difficultyHistorySize, currentHeight);
				Height startHeight(currentHeight - numDifficulties + 1);

				CATAPULT_LOG(info) << "Loading difficulties cache, starting at " << startHeight;
				LoadDifficulties(m_database, startHeight, numDifficulties, cache);
			}

		private:
			MongoDatabase m_database;
			uint64_t m_difficultyHistorySize;
		};
	}

	std::unique_ptr<ExternalCacheStorage> CreateMongoBlockDifficultyCacheStorage(
			MongoDatabase&& database,
			uint64_t difficultyHistorySize) {
		return std::make_unique<MongoBlockDifficultyCacheStorage>(std::move(database), difficultyHistorySize);
	}
}}}
