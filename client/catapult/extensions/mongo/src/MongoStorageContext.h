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
