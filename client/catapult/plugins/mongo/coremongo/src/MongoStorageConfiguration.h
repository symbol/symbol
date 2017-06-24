#pragma once
#include "MongoDatabase.h"

namespace catapult { namespace mongo { namespace plugins { class MongoBulkWriter; } } }

namespace catapult { namespace mongo { namespace plugins {

	/// Configuration for creating a mongo storage.
	class MongoStorageConfiguration {
	public:
		/// Creates an empty storage configuration.
		MongoStorageConfiguration() = default;

		/// Creates configuration for a mongodb-based storage connected to \a uri storing inside database \a databaseName
		/// with the specified bulk writer (\a pBulkWriter).
		MongoStorageConfiguration(
				const mongocxx::uri& uri,
				const std::string& databaseName,
				const std::shared_ptr<MongoBulkWriter>& pBulkWriter)
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
}}}
