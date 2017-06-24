#include "TransactionMapper.h"
#include "MapperUtils.h"
#include "src/MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		bsoncxx::document::value ToTransactionDbModel(
				const model::Transaction& transaction,
				const plugins::MongoTransactionMetadata& metadata,
				const plugins::MongoTransactionPlugin* pPlugin) {
			// transaction metadata
			bson_stream::document builder;
			builder << "_id" << metadata.ObjectId;
			builder << "meta"
					<< bson_stream::open_document
						<< "height" << ToInt64(metadata.Height)
						<< "hash" << ToBinary(metadata.EntityHash)
						<< "merkleComponentHash" << ToBinary(metadata.MerkleComponentHash)
						<< "index" << static_cast<int32_t>(metadata.Index)
					<< bson_stream::close_document;

			// transaction data
			builder << "transaction" << bson_stream::open_document;
			StreamVerifiableEntity(builder, transaction)
					<< "fee" << ToInt64(transaction.Fee)
					<< "deadline" << ToInt64(transaction.Deadline);

			if (pPlugin) {
				pPlugin->streamTransaction(builder, transaction);
			} else {
				const auto* pTransactionData = reinterpret_cast<const uint8_t*>(&transaction + 1);
				builder << "bin" << ToBinary(pTransactionData, transaction.Size - sizeof(model::Transaction));
			}

			builder << bson_stream::close_document;
			return builder << bson_stream::finalize;
		}
	}

	std::vector<bsoncxx::document::value> ToDbDocuments(
			const model::Transaction& transaction,
			const plugins::MongoTransactionMetadata& metadata,
			const plugins::MongoTransactionRegistry& transactionRegistry) {
		const auto* pPlugin = transactionRegistry.findPlugin(transaction.Type);

		std::vector<bsoncxx::document::value> documents;
		documents.push_back(ToTransactionDbModel(transaction, metadata, pPlugin));

		if (pPlugin) {
			auto dependentDocuments = pPlugin->extractDependentDocuments(transaction, metadata);
			documents.insert(documents.end(), dependentDocuments.cbegin(), dependentDocuments.cend());
		}

		return documents;
	}
}}}
