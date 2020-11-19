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

#pragma once
#include "MongoDatabase.h"
#include "MongoErrorPolicy.h"

namespace catapult { namespace mongo { class MongoBulkWriter; } }

namespace catapult { namespace mongo {

	/// Context for creating a mongo storage.
	class MongoStorageContext {
	public:
		/// Creates an empty storage context.
		MongoStorageContext() = default;

		/// Creates a storage context for a mongodb-based storage connected to \a uri storing inside database \a databaseName
		/// with the specified bulk writer (\a pBulkWriter) and error policy mode (\a errorPolicyMode).
		MongoStorageContext(
				const mongocxx::uri& uri,
				const std::string& databaseName,
				const std::shared_ptr<MongoBulkWriter>& pBulkWriter,
				MongoErrorPolicy::Mode errorPolicyMode)
				: m_connectionPool(uri)
				, m_databaseName(databaseName)
				, m_pBulkWriter(pBulkWriter)
				, m_errorPolicyMode(errorPolicyMode)
		{}

	public:
		/// Creates a mongo database connection.
		MongoDatabase createDatabaseConnection() {
			return MongoDatabase(m_connectionPool, m_databaseName);
		}

		/// Creates a mongo error policy for the collection with name \a collectionName.
		MongoErrorPolicy createCollectionErrorPolicy(const std::string& collectionName) {
			return MongoErrorPolicy(collectionName, m_errorPolicyMode);
		}

		/// Gets the bulk writer.
		MongoBulkWriter& bulkWriter() const {
			return *m_pBulkWriter;
		}

	private:
		mongocxx::pool m_connectionPool;
		std::string m_databaseName;
		std::shared_ptr<MongoBulkWriter> m_pBulkWriter;
		MongoErrorPolicy::Mode m_errorPolicyMode;
	};
}}
