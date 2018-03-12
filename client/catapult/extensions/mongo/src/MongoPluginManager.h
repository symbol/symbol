#pragma once
#include "ExternalCacheStorageBuilder.h"
#include "MongoStorageContext.h"
#include "MongoTransactionPlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins.h"

namespace catapult { namespace mongo {

	/// A manager for registering mongo plugins.
	class MongoPluginManager {
	public:
		/// Creates a new plugin manager around \a mongoContext and \a chainConfig.
		explicit MongoPluginManager(MongoStorageContext& mongoContext, const model::BlockChainConfiguration& chainConfig)
				: m_mongoContext(mongoContext)
				, m_chainConfig(chainConfig)
		{}

	public:
		/// Gets the mongo storage context.
		const MongoStorageContext& mongoContext() const {
			return m_mongoContext;
		}

		/// Gets the block chain configuration.
		const model::BlockChainConfiguration& chainConfig() const {
			return m_chainConfig;
		}

		/// Creates a mongo database connection.
		MongoDatabase createDatabaseConnection() {
			return m_mongoContext.createDatabaseConnection();
		}

	public:
		/// Adds support for a transaction described by \a pTransactionPlugin.
		void addTransactionSupport(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin) {
			m_transactionRegistry.registerPlugin(std::move(pTransactionPlugin));
		}

		/// Adds support for an external cache storage described by \a pStorage.
		template<typename TStorage>
		void addStorageSupport(std::unique_ptr<TStorage>&& pStorage) {
			m_storageBuilder.add(std::move(pStorage));
		}

	public:
		/// Gets the transaction registry.
		const MongoTransactionRegistry& transactionRegistry() const {
			return m_transactionRegistry;
		}

		/// Creates an external cache storage.
		std::unique_ptr<ExternalCacheStorage> createStorage() {
			return m_storageBuilder.build();
		}

	private:
		MongoStorageContext& m_mongoContext;
		model::BlockChainConfiguration m_chainConfig;
		MongoTransactionRegistry m_transactionRegistry;
		ExternalCacheStorageBuilder m_storageBuilder;
	};
}}

/// Entry point for registering a dynamic module with \a manager.
extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager);
