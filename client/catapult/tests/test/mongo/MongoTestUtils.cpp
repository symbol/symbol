#include "MongoTestUtils.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoPluginLoader.h"
#include "plugins/mongo/coremongo/src/MongoStorageConfiguration.h"
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "catapult/state/AccountState.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/mongo/mocks/MockTransactionMapper.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace test {

	std::string DatabaseName() {
		return "test-db";
	}

	mongocxx::uri DefaultDbUri() {
		return mongocxx::uri("mongodb://127.0.0.1:27017");
	}

	mongocxx::client CreateDbConnection() {
		return mongocxx::client(DefaultDbUri());
	}

	void PrepareDatabase(const std::string& dbName) {
		mongocxx::instance::current();
		auto connection = CreateDbConnection();

		connection[dbName].drop();

		auto database = connection[dbName];

		// indexes for the accounts collection
		auto accounts = database["accounts"];
		auto accountsAddressIndex = document{} << "account.address" << 1 << finalize;
		auto publicKeyIndex = document{} << "account.publicKey" << 1 << finalize;
		accounts.create_index(accountsAddressIndex.view(), mongocxx::options::index().unique(true));
		accounts.create_index(publicKeyIndex.view(), mongocxx::options::index());

		// indexes for the transactions collection
		auto transactions = database["transactions"];
		auto deadlineIndex = document{} << "transaction.deadline" << -1 << finalize;
		auto signerIndex = document{} << "transaction.signer" << 1 << "_id" << -1 << finalize;
		auto recipientIndex = document{} << "transaction.recipient" << 1 << "_id" << -1 << finalize;
		auto heightIndex = document{} << "meta.height" << -1 << finalize;
		transactions.create_index(deadlineIndex.view(), mongocxx::options::index());
		transactions.create_index(signerIndex.view(), mongocxx::options::index());
		transactions.create_index(recipientIndex.view(), mongocxx::options::index());
		transactions.create_index(heightIndex.view(), mongocxx::options::index());
	}

	bsoncxx::document::value CreateFilter(const std::shared_ptr<state::AccountState>& pAccountState) {
		auto filter = document()
				<< "account.address"
				<< open_document
					<< "$eq" << mongo::mappers::ToBinary(pAccountState->Address)
				<< close_document
				<< finalize;
		return filter;
	}

	std::shared_ptr<mongo::plugins::MongoStorageConfiguration> CreateDefaultMongoStorageConfiguration(const std::string& dbName) {
		auto pWriter = mongo::plugins::MongoBulkWriter::Create(test::DefaultDbUri(), dbName, test::CreateStartedIoServiceThreadPool(8));
		return std::make_shared<mongo::plugins::MongoStorageConfiguration>(test::DefaultDbUri(), dbName, pWriter);
	}

	std::shared_ptr<mongo::plugins::MongoTransactionRegistry> CreateDefaultMongoTransactionRegistry() {
		auto pRegistry = std::make_shared<mongo::plugins::MongoTransactionRegistry>();
		pRegistry->registerPlugin(mocks::CreateMockTransactionMongoPlugin());
		return pRegistry;
	}
}}
