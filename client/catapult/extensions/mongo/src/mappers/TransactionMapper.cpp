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

#include "TransactionMapper.h"
#include "MapperUtils.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		void StreamAddresses(bson_stream::document& builder, const model::UnresolvedAddressSet& addresses) {
			auto addressesArray = builder << "addresses" << bson_stream::open_array;
			for (const auto& address : addresses)
				addressesArray << ToBinary(address);

			addressesArray << bson_stream::close_array;
		}

		bsoncxx::document::value ToTransactionDbModel(
				const model::Transaction& transaction,
				const MongoTransactionMetadata& metadata,
				const MongoTransactionPlugin* pPlugin) {
			// transaction metadata
			bson_stream::document builder;
			builder << "_id" << metadata.ObjectId;
			builder
					<< "meta" << bson_stream::open_document
						<< "height" << ToInt64(metadata.Height)
						<< "hash" << ToBinary(metadata.EntityHash)
						<< "merkleComponentHash" << ToBinary(metadata.MerkleComponentHash)
						<< "index" << static_cast<int32_t>(metadata.Index);

			StreamAddresses(builder, metadata.Addresses);
			builder << bson_stream::close_document;

			// transaction data
			builder << "transaction" << bson_stream::open_document;
			StreamVerifiableEntity(builder, transaction)
					<< "maxFee" << ToInt64(transaction.MaxFee)
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
			const MongoTransactionMetadata& metadata,
			const MongoTransactionRegistry& transactionRegistry) {
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
