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
#include "MongoDatabase.h"

namespace catapult { namespace mongo { class MongoBulkWriter; } }

namespace catapult { namespace mongo {

	/// Context for creating a mongo storage.
	class MongoStorageContext {
	public:
		/// Creates an empty storage context.
		MongoStorageContext() = default;

		/// Creates a storage context for a mongodb-based storage connected to \a uri storing inside database \a databaseName
		/// with the specified bulk writer (\a pBulkWriter).
		MongoStorageContext(const mongocxx::uri& uri, const std::string& databaseName, const std::shared_ptr<MongoBulkWriter>& pBulkWriter)
				: m_connectionPool(uri)
				, m_databaseName(databaseName)
				, m_pBulkWriter(pBulkWriter)
		{}

	public:
		/// Creates a mongo database connection.
		MongoDatabase createDatabaseConnection() {
			return MongoDatabase(m_connectionPool, m_databaseName);
		}

		/// Gets the bulk writer.
		MongoBulkWriter& bulkWriter() const {
			return *m_pBulkWriter;
		}

	private:
		mongocxx::pool m_connectionPool;
		std::string m_databaseName;
		std::shared_ptr<MongoBulkWriter> m_pBulkWriter;
	};
}}
