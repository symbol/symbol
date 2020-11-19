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

#include "MongoChainScoreProvider.h"
#include "MongoChainStatisticUtils.h"
#include "mappers/MapperUtils.h"
#include "catapult/model/ChainScore.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		class MongoChainScoreProvider final : public ChainScoreProvider {
		public:
			explicit MongoChainScoreProvider(MongoStorageContext& context)
					: m_database(context.createDatabaseConnection())
					, m_errorPolicy(context.createCollectionErrorPolicy("chainStatistic"))
			{}

		public:
			void saveScore(const model::ChainScore& chainScore) override {
				auto scoreArray = chainScore.toArray();
				auto scoreValue = document()
						<< "$set" << open_document
							<< "current.scoreHigh" << static_cast<int64_t>(scoreArray[0])
							<< "current.scoreLow" << static_cast<int64_t>(scoreArray[1])
						<< close_document
						<< finalize;

				auto result = TrySetChainStatisticDocument(m_database, scoreValue.view());
				m_errorPolicy.checkUpserted(1, result, "chain score");
			}

		private:
			MongoDatabase m_database;
			MongoErrorPolicy m_errorPolicy;
		};
	}

	std::unique_ptr<ChainScoreProvider> CreateMongoChainScoreProvider(MongoStorageContext& context) {
		return std::make_unique<MongoChainScoreProvider>(context);
	}
}}
