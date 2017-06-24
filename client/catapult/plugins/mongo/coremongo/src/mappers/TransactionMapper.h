#pragma once
#include "catapult/types.h"
#include <vector>

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document { class value; }
	}
}

namespace catapult {
	namespace model { struct Transaction; }
	namespace mongo { namespace plugins {
		struct MongoTransactionMetadata;
		class MongoTransactionRegistry;
	}}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a \a transaction with \a metadata to representative db documents using \a transactionRegistry for mapping
	/// derived transaction types.
	std::vector<bsoncxx::document::value> ToDbDocuments(
			const model::Transaction& transaction,
			const plugins::MongoTransactionMetadata& metadata,
			const plugins::MongoTransactionRegistry& transactionRegistry);
}}}
