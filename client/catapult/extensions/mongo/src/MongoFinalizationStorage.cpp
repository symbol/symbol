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

#include "MongoFinalizationStorage.h"
#include "mappers/BlockMapper.h"
#include "mappers/MapperUtils.h"
#include "catapult/exceptions.h"
#include <mongocxx/client.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Collection_Name = "finalizedBlocks";

		class MongoFinalizationStorage final : public subscribers::FinalizationSubscriber {
		public:
			MongoFinalizationStorage(MongoStorageContext& context)
					: m_context(context)
					, m_database(m_context.createDatabaseConnection())
			{}

		public:
			void notifyFinalizedBlock(Height height, const Hash256& hash, FinalizationPoint point) override {
				auto blocks = m_database[Collection_Name];
				auto dbFinalizedBlock = mappers::ToDbModel(height, hash, point);

				// since this storage is not updated during chain sync, it doesn't need to support recovery
				auto result = blocks.insert_one(dbFinalizedBlock.view()).value().result();
				if (0 == result.inserted_count())
					CATAPULT_THROW_RUNTIME_ERROR("notifyFinalizedBlock failed: finalized block was not inserted");
			}

		private:
			MongoStorageContext& m_context;
			MongoDatabase m_database;
		};
	}

	std::unique_ptr<subscribers::FinalizationSubscriber> CreateMongoFinalizationStorage(MongoStorageContext& context) {
		return std::make_unique<MongoFinalizationStorage>(context);
	}
}}
