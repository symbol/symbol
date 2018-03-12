#include "ExtensionManager.h"
#include "BasicServerHooks.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace extensions {

	ExtensionManager::ExtensionManager() {
		std::string prefix = "catapult.";
		for (const auto& name : { "coresystem", "plugins.signature" })
			m_systemPluginNames.push_back(prefix + name);
	}

	void ExtensionManager::registerSystemPlugin(const std::string& name) {
		m_systemPluginNames.push_back(name);
	}

	void ExtensionManager::addPreLoadHandler(const CacheConsumer& handler) {
		m_preLoadHandlers.push_back(handler);
	}

	void ExtensionManager::setNetworkTimeSupplier(const NetworkTimeSupplier& supplier) {
		SetOnce(m_networkTimeSupplier, supplier);
	}

	void ExtensionManager::setBlockChainStorage(std::unique_ptr<BlockChainStorage>&& pBlockChainStorage) {
		if (m_pBlockChainStorage)
			CATAPULT_THROW_INVALID_ARGUMENT("block chain storage cannot be set more than once");

		m_pBlockChainStorage = std::move(pBlockChainStorage);
	}

	void ExtensionManager::addServiceRegistrar(std::unique_ptr<ServiceRegistrar>&& pServiceRegistrar) {
		m_serviceRegistrars.push_back(std::move(pServiceRegistrar));
	}

	const std::vector<std::string>& ExtensionManager::systemPluginNames() const {
		return m_systemPluginNames;
	}

	ExtensionManager::CacheConsumer ExtensionManager::preLoadHandler() const {
		return AggregateConsumers(m_preLoadHandlers);
	}

	ExtensionManager::NetworkTimeSupplier ExtensionManager::networkTimeSupplier() const {
		return m_networkTimeSupplier ? m_networkTimeSupplier : &utils::NetworkTime;
	}

	std::unique_ptr<BlockChainStorage> ExtensionManager::createBlockChainStorage() {
		if (!m_pBlockChainStorage)
			CATAPULT_THROW_RUNTIME_ERROR("BlockChainStorage is not configured");

		return std::move(m_pBlockChainStorage);
	}

	void ExtensionManager::registerServices(ServiceLocator& locator, ServiceState& state) {
		// sort the registrars based on their annnounced phases
		std::sort(m_serviceRegistrars.begin(), m_serviceRegistrars.end(), [](const auto& pLhs, const auto& pRhs) {
			return utils::to_underlying_type(pLhs->info().Phase) < utils::to_underlying_type(pRhs->info().Phase);
		});

		// register counters first
		for (const auto& pRegistrar : m_serviceRegistrars)
			pRegistrar->registerServiceCounters(locator);

		// register services second
		for (const auto& pRegistrar : m_serviceRegistrars) {
			CATAPULT_LOG(debug) << "registering " << pRegistrar->info().Name << " services";
			pRegistrar->registerServices(locator, state);
		}

		// services are registered, no need to keep registrars around
		m_serviceRegistrars.clear();
	}
}}
