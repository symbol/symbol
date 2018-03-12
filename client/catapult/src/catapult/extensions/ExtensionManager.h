#pragma once
#include "BlockChainStorage.h"
#include "ServiceRegistrar.h"
#include "catapult/functions.h"
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult { namespace cache { class CatapultCache; } }

namespace catapult { namespace extensions {

	/// A manager for registering extensions.
	class ExtensionManager {
	public:
		/// Consumer that accepts a constant catapult cache.
		using CacheConsumer = consumer<const cache::CatapultCache&>;

		/// Supplier that returns the network time.
		using NetworkTimeSupplier = supplier<Timestamp>;

	public:
		/// Creates a manager.
		ExtensionManager();

	public:
		/// Registers a system plugin with \a name.
		void registerSystemPlugin(const std::string& name);

		/// Adds a \a handler that is called after the catapult cache is created but before the block chain is loaded.
		void addPreLoadHandler(const CacheConsumer& handler);

		/// Registers a network time \a supplier.
		void setNetworkTimeSupplier(const NetworkTimeSupplier& supplier);

		/// Registers a block chain storage provider (\a pBlockChainStorage).
		void setBlockChainStorage(std::unique_ptr<BlockChainStorage>&& pBlockChainStorage);

		/// Adds a service registrar (\a pServiceRegistrar).
		void addServiceRegistrar(std::unique_ptr<ServiceRegistrar>&& pServiceRegistrar);

	public:
		/// Gets the system plugin names.
		const std::vector<std::string>& systemPluginNames() const;

		/// Gets the pre load handler.
		CacheConsumer preLoadHandler() const;

		/// Gets the network time supplier.
		NetworkTimeSupplier networkTimeSupplier() const;

		/// Creates the block chain storage.
		std::unique_ptr<BlockChainStorage> createBlockChainStorage();

		/// Registers all services by forwarding \a locator and \a state.
		void registerServices(ServiceLocator& locator, ServiceState& state);

	private:
		std::vector<std::string> m_systemPluginNames;
		std::vector<CacheConsumer> m_preLoadHandlers;
		NetworkTimeSupplier m_networkTimeSupplier;
		std::unique_ptr<BlockChainStorage> m_pBlockChainStorage;
		std::vector<std::unique_ptr<ServiceRegistrar>> m_serviceRegistrars;
	};
}}
