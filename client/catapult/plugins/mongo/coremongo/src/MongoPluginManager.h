#pragma once
#include "ExternalCacheStorageBuilder.h"
#include "MongoStorageConfiguration.h"
#include "MongoTransactionPlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins.h"

namespace catapult { namespace mongo { namespace plugins {

	/// A manager for registering mongo plugins.
	class MongoPluginManager {
	public:
		/// Creates a new plugin manager around \a mongoConfig and \a chainConfig.
		explicit MongoPluginManager(
				MongoStorageConfiguration& mongoConfig,
				const model::BlockChainConfiguration& chainConfig)
				: m_mongoConfig(mongoConfig)
				, m_chainConfig(chainConfig)
		{}

	public:
		/// Gets the mongo storage configuration.
		const MongoStorageConfiguration& mongoConfig() const {
			return m_mongoConfig;
		}

		/// Gets the block chain configuration.
		const model::BlockChainConfiguration& chainConfig() const {
			return m_chainConfig;
		}

		/// Creates a mongo database connection.
		MongoDatabase createDatabaseConnection() {
			return m_mongoConfig.createDatabaseConnection();
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
		MongoStorageConfiguration& m_mongoConfig;
		model::BlockChainConfiguration m_chainConfig;
		MongoTransactionRegistry m_transactionRegistry;
		ExternalCacheStorageBuilder m_storageBuilder;
	};
}}}

/// Entry point for registering a dynamic module with \a manager.
extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::plugins::MongoPluginManager& manager);
