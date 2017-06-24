#include "plugins/mongo/coremongo/src/ExternalCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoDatabasePlugin.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/local/ConfigurationUtils.h"
#include "catapult/local/api/DatabaseConfiguration.h"
#include "catapult/local/api/LocalNode.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/server/ServerMain.h"
#include "catapult/thread/MultiServicePool.h"
#include <mongocxx/instance.hpp>

using namespace catapult::mongo::plugins;

namespace {
	std::shared_ptr<const MongoTransactionRegistry> CreateTransactionRegistry(
			std::shared_ptr<catapult::mongo::plugins::MongoPluginManager>& pPluginManager,
			const std::string& directory,
			const std::unordered_set<std::string>& pluginNames) {
		std::vector<catapult::plugins::PluginModule> modules;
		LoadPluginByName(*pPluginManager, modules, directory, "catapult.plugins.mongo.coremongo");
		for (const auto& pluginName : pluginNames)
			LoadPluginByName(*pPluginManager, modules, directory, pluginName);

		return std::shared_ptr<const MongoTransactionRegistry>(
				&pPluginManager->transactionRegistry(),
				[pPluginManager, modules = std::move(modules)](const auto*) mutable {
					// destroy the modules after the manager
					pPluginManager.reset();
					modules.clear();
				});
	}
}

int main(int argc, const char** argv) {
	using namespace catapult;
	mongocxx::instance::current();

	return server::ServerMain(argc, argv, [argc, argv](auto&& config, const auto& keyPair) {
		// load db configuration
		auto dbConfig = local::api::DatabaseConfiguration::LoadFromPath(server::GetResourcesPath(argc, argv));
		auto dbUri = mongocxx::uri(dbConfig.DatabaseUri);
		const auto& dbName = dbConfig.DatabaseName;

		// create mongo writer
		auto numServicePoolThreads = thread::MultiServicePool::DefaultPoolConcurrency();
		auto pMultiServicePool = std::make_unique<thread::MultiServicePool>(numServicePoolThreads, "api");
		auto numWriterThreads = std::min(std::thread::hardware_concurrency(), dbConfig.MaxWriterThreads);
		auto pBulkWriterPool = pMultiServicePool->pushIsolatedPool(numWriterThreads, "bulk writer");
		auto pMongoBulkWriter = MongoBulkWriter::Create(
				dbUri,
				dbName,
				// pass in a non-owning shared_ptr so that the writer does not keep the bulk writer pool alive during shutdown
				std::shared_ptr<thread::IoServiceThreadPool>(pBulkWriterPool.get(), [](const auto*) {}));

		// create mongo (block and state) storage
		auto pMongoConfig = std::make_shared<MongoStorageConfiguration>(dbUri, dbName, pMongoBulkWriter);
		auto pPluginManager = std::make_shared<MongoPluginManager>(*pMongoConfig, config.BlockChain);
		auto pTransactionRegistry = CreateTransactionRegistry(pPluginManager, config.User.PluginsDirectory, dbConfig.PluginNames);

		auto pMongoProvider = std::make_shared<MongoDbStorage>(pMongoConfig, pTransactionRegistry);
		auto pMongoAggregate = std::make_unique<MongoAggregateStorage>(
				decltype(pMongoProvider)(pMongoProvider),
				std::make_shared<io::FileBasedStorage>(config.User.DataDirectory),
				config.BlockChain.MaxRollbackBlocks);

		// create mongo (cache) storage
		auto pExternalCacheStorage = pPluginManager->createStorage();

		// create mongo (ut) storage
		auto pMemoryUnconfirmedTransactionsCache = std::make_shared<cache::MemoryUtCache>(
				local::GetUnconfirmedTransactionsCacheOptions(config.Node));
		auto pHybridUnconfirmedTransactionsCache = std::make_unique<StorageAwareUtCache>(
				pMemoryUnconfirmedTransactionsCache,
				std::make_shared<MongoDbTransactionStorage>(pMongoConfig, pTransactionRegistry));

		// create the local node
		return local::api::CreateLocalNode(
				keyPair,
				std::move(config),
				std::move(pMultiServicePool),
				std::move(pMongoAggregate),
				std::move(pMongoProvider),
				std::move(pExternalCacheStorage),
				[pMemoryUnconfirmedTransactionsCache]() { return pMemoryUnconfirmedTransactionsCache->view(); },
				std::move(pHybridUnconfirmedTransactionsCache));
	});
}
