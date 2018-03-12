#pragma once
#include "MapperInclude.h"
#include <vector>

namespace catapult {
	namespace model { struct Transaction; }
	namespace mongo {
		struct MongoTransactionMetadata;
		class MongoTransactionRegistry;
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a \a transaction with \a metadata to representative db documents using \a transactionRegistry for mapping
	/// derived transaction types.
	std::vector<bsoncxx::document::value> ToDbDocuments(
			const model::Transaction& transaction,
			const MongoTransactionMetadata& metadata,
			const MongoTransactionRegistry& transactionRegistry);
}}}
