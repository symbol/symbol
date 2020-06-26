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

#include "src/ApiStateChangeSubscriber.h"
#include "src/CoreMongo.h"
#include "src/DatabaseConfiguration.h"
#include "src/MongoBlockStorage.h"
#include "src/MongoBulkWriter.h"
#include "src/MongoChainScoreProvider.h"
#include "src/MongoFinalizationStorage.h"
#include "src/MongoPluginLoader.h"
#include "src/MongoPluginManager.h"
#include "src/MongoPtStorage.h"
#include "src/MongoTransactionStatusStorage.h"
#include "src/MongoTransactionStorage.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/extensions/RootedService.h"
#include "catapult/io/BlockStorageChangeSubscriber.h"
#include <mongocxx/instance.hpp>

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Ut_Collection_Name = "unconfirmedTransactions";
		constexpr auto Pt_Collection_Name = "partialTransactions";

		std::shared_ptr<const MongoTransactionRegistry> CreateTransactionRegistry(
				std::shared_ptr<mongo::MongoPluginManager>& pPluginManager,
				const std::string& directory,
				const std::unordered_set<std::string>& pluginNames) {
			RegisterCoreMongoSystem(*pPluginManager);

			std::vector<plugins::PluginModule> modules;
			for (const auto& pluginName : pluginNames)
				LoadPluginByName(*pPluginManager, modules, directory, pluginName);

			// need to use a shared_ptr to tie the lifetime of the modules to that of the registry
			return std::shared_ptr<const MongoTransactionRegistry>(
					&pPluginManager->transactionRegistry(),
					[pPluginManager, modules = std::move(modules)](const auto*) mutable {
						// destroy the modules after the manager
						pPluginManager.reset();
						modules.clear();
					});
		}

		void EmptyCollection(MongoStorageContext& context, const std::string& collectionName) {
			auto database = context.createDatabaseConnection();
			auto collection = database[collectionName];
			collection.delete_many({});
		}

		class MongoServices {
		public:
			MongoServices(
					const std::shared_ptr<const MongoStorageContext>& pContext,
					const std::shared_ptr<const MongoTransactionRegistry>& pRegistry)
					: m_pContext(pContext)
					, m_pRegistry(pRegistry)
			{}

		private:
			std::shared_ptr<const MongoStorageContext> m_pContext;
			std::shared_ptr<const MongoTransactionRegistry> m_pRegistry;
		};

		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
			mongocxx::instance::current();

			// load db configuration
			auto dbConfig = DatabaseConfiguration::LoadFromPath(bootstrapper.resourcesPath());
			auto dbUri = mongocxx::uri(dbConfig.DatabaseUri);
			const auto& dbName = dbConfig.DatabaseName;

			// create mongo writer
			// keep the minimum high enough in order to avoid deadlock while waiting for mongo operations due to blocking io threads
			auto numWriterThreads = std::max(4u, std::min(std::thread::hardware_concurrency(), dbConfig.MaxWriterThreads));
			auto* pBulkWriterPool = bootstrapper.pool().pushIsolatedPool("bulk writer", numWriterThreads);
			auto pMongoBulkWriter = MongoBulkWriter::Create(dbUri, dbName, *pBulkWriterPool);

			// create transaction registry
			const auto& config = bootstrapper.config();
			auto mongoErrorPolicyMode = extensions::ProcessDisposition::Recovery == bootstrapper.disposition()
					? MongoErrorPolicy::Mode::Idempotent
					: MongoErrorPolicy::Mode::Strict;
			auto pMongoContext = std::make_shared<MongoStorageContext>(dbUri, dbName, pMongoBulkWriter, mongoErrorPolicyMode);
			auto pPluginManager = std::make_shared<MongoPluginManager>(*pMongoContext, config.BlockChain.Network.Identifier);
			auto pTransactionRegistry = CreateTransactionRegistry(pPluginManager, config.User.PluginsDirectory, dbConfig.Plugins);

			// create mongo chain score provider and mongo (cache) storage
			auto pChainScoreProvider = CreateMongoChainScoreProvider(*pMongoContext);
			auto pExternalCacheStorage = pPluginManager->createStorage();

			// add a dummy service for extending service lifetimes
			bootstrapper.extensionManager().addServiceRegistrar(extensions::CreateRootedServiceRegistrar(
					std::make_shared<MongoServices>(pMongoContext, pTransactionRegistry),
					"mongo.services",
					extensions::ServiceRegistrarPhase::Initial_With_Modules));

			// add a pre load handler for initializing (nemesis) storage
			// (pPluginManager is kept alive by pTransactionRegistry)
			auto pMongoBlockStorage = CreateMongoBlockStorage(*pMongoContext, *pTransactionRegistry, pPluginManager->receiptRegistry());

			// empty unconfirmed and partial transactions collections
			EmptyCollection(*pMongoContext, Ut_Collection_Name);
			EmptyCollection(*pMongoContext, Pt_Collection_Name);

			// register subscriptions
			bootstrapper.subscriptionManager().addBlockChangeSubscriber(
					io::CreateBlockStorageChangeSubscriber(std::move(pMongoBlockStorage)));
			bootstrapper.subscriptionManager().addPtChangeSubscriber(CreateMongoPtStorage(*pMongoContext, *pTransactionRegistry));
			bootstrapper.subscriptionManager().addUtChangeSubscriber(
					CreateMongoTransactionStorage(*pMongoContext, *pTransactionRegistry, Ut_Collection_Name));
			bootstrapper.subscriptionManager().addFinalizationSubscriber(CreateMongoFinalizationStorage(*pMongoContext));
			bootstrapper.subscriptionManager().addStateChangeSubscriber(std::make_unique<ApiStateChangeSubscriber>(
					std::move(pChainScoreProvider),
					std::move(pExternalCacheStorage)));
			bootstrapper.subscriptionManager().addTransactionStatusSubscriber(CreateMongoTransactionStatusStorage(*pMongoContext));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::mongo::RegisterExtension(bootstrapper);
}
