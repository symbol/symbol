#pragma once
#include <mongocxx/client.hpp>

namespace catapult { namespace mongo {

	/// Upserts the chain info document in \a database with \a upsertDoc.
	void SetChainInfoDocument(mongocxx::database& database, const bsoncxx::document::view& upsertDoc);

	/// Gets the chain info document from \a database.
	bsoncxx::document::value GetChainInfoDocument(const mongocxx::database& database);
}}
