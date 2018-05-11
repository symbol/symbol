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

#include "MongoChainScoreProvider.h"
#include "MongoChainInfoUtils.h"
#include "mappers/MapperUtils.h"
#include "catapult/model/ChainScore.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		void SetScore(mongocxx::database& database, const model::ChainScore& score) {
			auto scoreArray = score.toArray();
			auto scoreValue = document()
					<< "$set"
					<< open_document
						<< "scoreHigh" << static_cast<int64_t>(scoreArray[0])
						<< "scoreLow" << static_cast<int64_t>(scoreArray[1])
					<< close_document
					<< finalize;

			SetChainInfoDocument(database, scoreValue.view());
		}

		class MongoChainScoreProvider final : public ChainScoreProvider {
		public:
			explicit MongoChainScoreProvider(MongoStorageContext& context)
					: m_database(context.createDatabaseConnection())
			{}

		public:
			void saveScore(const model::ChainScore& chainScore) override {
				SetScore(m_database, chainScore);
			}

			model::ChainScore loadScore() const override {
				auto chainInfoDocument = GetChainInfoDocument(m_database);
				if (mappers::IsEmptyDocument(chainInfoDocument))
					return model::ChainScore();

				auto scoreLow = mappers::GetUint64OrDefault(chainInfoDocument.view(), "scoreLow", 0);
				auto scoreHigh = mappers::GetUint64OrDefault(chainInfoDocument.view(), "scoreHigh", 0);
				return model::ChainScore(scoreHigh, scoreLow);
			}

		private:
			MongoDatabase m_database;
		};
	}

	std::unique_ptr<ChainScoreProvider> CreateMongoChainScoreProvider(MongoStorageContext& context) {
		return std::make_unique<MongoChainScoreProvider>(context);
	}
}}
