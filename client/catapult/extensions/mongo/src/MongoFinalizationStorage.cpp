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
#include "BulkWriteResult.h"
#include "mappers/BlockMapper.h"
#include "mappers/MapperUtils.h"
#include <mongocxx/client.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Collection_Name = "finalizedBlocks";

		auto CreateFilter(const model::FinalizationRound& round) {
			auto filter = document()
					<< "block.finalizationEpoch" << mongo::mappers::ToInt32(round.Epoch)
					<< "block.finalizationPoint" << mongo::mappers::ToInt32(round.Point)
					<< finalize;
			return filter;
		}

		class MongoFinalizationStorage final : public subscribers::FinalizationSubscriber {
		public:
			MongoFinalizationStorage(MongoStorageContext& context)
					: m_context(context)
					, m_database(m_context.createDatabaseConnection())
					, m_errorPolicy(m_context.createCollectionErrorPolicy(Collection_Name))
			{}

		public:
			void notifyFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) override {
				auto blocks = m_database[Collection_Name];
				auto dbFinalizedBlock = mappers::ToDbModel(round, height, hash);

				mongocxx::options::replace replace_op;
				replace_op.upsert(true);

				auto result = blocks.replace_one(CreateFilter(round), dbFinalizedBlock.view(), replace_op).value().result();
				m_errorPolicy.checkUpserted(1, BulkWriteResult(result), "finalized block");
			}

		private:
			MongoStorageContext& m_context;
			MongoDatabase m_database;
			MongoErrorPolicy m_errorPolicy;
		};
	}

	std::unique_ptr<subscribers::FinalizationSubscriber> CreateMongoFinalizationStorage(MongoStorageContext& context) {
		return std::make_unique<MongoFinalizationStorage>(context);
	}
}}
