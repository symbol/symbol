#include "MongoChainInfoUtils.h"
#include "catapult/exceptions.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	void SetChainInfoDocument(mongocxx::database& database, const bsoncxx::document::view& upsertDoc) {
		auto collection = database["chainInfo"];
		auto result = collection.update_one({}, upsertDoc, mongocxx::options::update().upsert(true)).get().result();
		if (0 == result.modified_count() && 0 == result.upserted_count())
			CATAPULT_THROW_RUNTIME_ERROR("SetChainInfoDocument failed: no document upserted or modified");
	}

	bsoncxx::document::value GetChainInfoDocument(const mongocxx::database& database) {
		auto collection = database["chainInfo"];
		auto matchedDocument = collection.find_one({});
		if (matchedDocument.is_initialized())
			return matchedDocument.get();

		return bsoncxx::document::value(nullptr, 0, [](auto*) {});
	}
}}
