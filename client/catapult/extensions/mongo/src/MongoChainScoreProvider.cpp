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
