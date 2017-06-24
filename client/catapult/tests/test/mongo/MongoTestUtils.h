#pragma once
#include <memory>
#include <string>

namespace catapult {
	namespace mongo {
		namespace plugins {
			class MongoStorageConfiguration;
			class MongoTransactionRegistry;
		}
	}
	namespace state { struct AccountState; }
}

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document {
			class value;
		}
	}
}

namespace mongocxx {
	inline namespace v_noabi {
		class client;
		class uri;
	}
}

namespace catapult { namespace test {

	/// Returns the database name for tests.
	std::string DatabaseName();

	/// Returns the default db uri for tests.
	mongocxx::uri DefaultDbUri();

	/// Creates a mongo database connection.
	mongocxx::client CreateDbConnection();

	/// Prepares the mongo database with name \a dbName by
	/// 1) dropping and recreating it (in order to have empty collections).
	/// 2) adding indexes to the accounts and transactions collections.
	void PrepareDatabase(const std::string& dbName);

	/// Creates a filter for the given \a pAccountState.
	bsoncxx::document::value CreateFilter(const std::shared_ptr<state::AccountState>& pAccountState);

	/// Creates a default mongo storage configuration for database \a dbName.
	std::shared_ptr<mongo::plugins::MongoStorageConfiguration> CreateDefaultMongoStorageConfiguration(const std::string& dbName);

	/// Creates a default mongo transaction registry that supports mock transactions.
	std::shared_ptr<mongo::plugins::MongoTransactionRegistry> CreateDefaultMongoTransactionRegistry();

	/// Database mixin for preparing mongo database access.
	struct PrepareDatabaseMixin {
		/// Creates a database mixin around \a dbName.
		explicit PrepareDatabaseMixin(const std::string& dbName) {
			PrepareDatabase(dbName);
		}
	};
}}
