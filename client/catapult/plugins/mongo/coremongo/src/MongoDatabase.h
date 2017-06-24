#pragma once
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <string>

namespace catapult { namespace mongo { namespace plugins {

	/// Represents a mongo database.
	class MongoDatabase {
	public:
		/// Creates a mongo database around \a connectionPool and \a databaseName.
		explicit MongoDatabase(mongocxx::pool& connectionPool, const std::string& databaseName)
				: m_pConnection(connectionPool.acquire())
				, m_database(m_pConnection->database(databaseName))
		{}

	public:
		/// Gets a const reference to the underlying mongocxx database.
		operator const mongocxx::database&() const {
			return m_database;
		}

		/// Gets a reference to the underlying mongocxx database.
		operator mongocxx::database&() {
			return m_database;
		}

		/// Gets the const mongocxx collection with name \a collectionName.
		const mongocxx::collection operator[](const std::string& collectionName) const {
			return m_database[collectionName];
		}

		/// Gets the mongocxx collection with name \a collectionName.
		mongocxx::collection operator[](const std::string& collectionName) {
			return m_database[collectionName];
		}

	private:
		mongocxx::pool::entry m_pConnection;
		mongocxx::database m_database;
	};
}}}
