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

#pragma once
#include "MongoTransactionMetadata.h"
#include "catapult/model/TransactionRegistry.h"
#include "catapult/plugins.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>

namespace catapult {
	namespace model {
		struct EmbeddedTransaction;
		struct Transaction;
	}
}

namespace catapult { namespace mongo {

	/// Typed mongo transaction plugin.
	template<typename TTransaction>
	class PLUGIN_API_DEPENDENCY MongoTransactionPluginT {
	public:
		virtual ~MongoTransactionPluginT() = default;

	public:
		/// Gets the transaction entity type.
		virtual model::EntityType type() const = 0;

		/// Streams \a transaction to \a builder.
		virtual void streamTransaction(bsoncxx::builder::stream::document& builder, const TTransaction& transaction) const = 0;
	};

	/// Embedded mongo transaction plugin.
	class PLUGIN_API_DEPENDENCY EmbeddedMongoTransactionPlugin : public MongoTransactionPluginT<model::EmbeddedTransaction> {};

	/// Mongo transaction plugin.
	class PLUGIN_API_DEPENDENCY MongoTransactionPlugin : public MongoTransactionPluginT<model::Transaction> {
	public:
		/// Extracts dependent documents from \a transaction given the associated \a metadata.
		/// \note The document representing the transaction is created separately via the streamTransaction() call.
		virtual std::vector<bsoncxx::document::value> extractDependentDocuments(
				const model::Transaction& transaction,
				const MongoTransactionMetadata& metadata) const = 0;

		/// \c true if this transaction type supports embedding.
		virtual bool supportsEmbedding() const = 0;

		/// Gets the corresponding embedded plugin if supportsEmbedding() is \c true.
		virtual const EmbeddedMongoTransactionPlugin& embeddedPlugin() const = 0;
	};

	/// Registry of mongo transaction plugins.
	class MongoTransactionRegistry : public model::TransactionRegistryT<MongoTransactionPlugin> {};
}}
