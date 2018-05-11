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

#include "LocalNodeBootstrapper.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/utils/Logging.h"
#include <boost/exception_ptr.hpp>

namespace catapult { namespace extensions {

	namespace {
		plugins::StorageConfiguration CreateStorageConfiguration(const config::LocalNodeConfiguration& config) {
			plugins::StorageConfiguration storageConfig;
			storageConfig.PreferCacheDatabase = config.Node.ShouldUseCacheDatabaseStorage;
			storageConfig.CacheDatabaseDirectory = (boost::filesystem::path(config.User.DataDirectory) / "statedb").generic_string();
			return storageConfig;
		}
	}

	LocalNodeBootstrapper::LocalNodeBootstrapper(
			const config::LocalNodeConfiguration& config,
			const std::string& resourcesPath,
			const std::string& servicePoolName)
			: m_config(std::move(config))
			, m_resourcesPath(resourcesPath)
			, m_pMultiServicePool(std::make_unique<thread::MultiServicePool>(
					servicePoolName,
					thread::MultiServicePool::DefaultPoolConcurrency(),
					m_config.Node.ShouldUseSingleThreadPool
							? thread::MultiServicePool::IsolatedPoolMode::Disabled
							: thread::MultiServicePool::IsolatedPoolMode::Enabled))
			, m_subscriptionManager(config)
			, m_pluginManager(m_config.BlockChain, CreateStorageConfiguration(config))
	{}

	const config::LocalNodeConfiguration& LocalNodeBootstrapper::config() const {
		return m_config;
	}

	const std::string& LocalNodeBootstrapper::resourcesPath() const {
		return m_resourcesPath;
	}

	const std::vector<ionet::Node>& LocalNodeBootstrapper::staticNodes() const {
		return m_nodes;
	}

	thread::MultiServicePool& LocalNodeBootstrapper::pool() {
		return *m_pMultiServicePool;
	}

	ExtensionManager& LocalNodeBootstrapper::extensionManager() {
		return m_extensionManager;
	}

	subscribers::SubscriptionManager& LocalNodeBootstrapper::subscriptionManager() {
		return m_subscriptionManager;
	}

	plugins::PluginManager& LocalNodeBootstrapper::pluginManager() {
		return m_pluginManager;
	}

	namespace {
		using RegisterExtensionFunc = void (*)(LocalNodeBootstrapper&);

		void LoadExtension(const plugins::PluginModule& module, LocalNodeBootstrapper& bootstrapper) {
			auto registerExtension = module.symbol<RegisterExtensionFunc>("RegisterExtension");

			try {
				registerExtension(bootstrapper);
			} catch (...) {
				// since the module will be unloaded after this function exits, throw a copy of the exception that
				// is not dependent on the (soon to be unloaded) module
				auto exInfo = boost::diagnostic_information(boost::current_exception());
				CATAPULT_THROW_AND_LOG_0(plugins::plugin_load_error, exInfo.c_str());
			}
		}
	}

	void LocalNodeBootstrapper::loadExtensions() {
		for (const auto& extension : m_config.Node.Extensions) {
			m_extensionModules.emplace_back(m_config.User.PluginsDirectory, extension);

			CATAPULT_LOG(info) << "registering dynamic extension " << extension;
			LoadExtension(m_extensionModules.back(), *this);
		}
	}

	void LocalNodeBootstrapper::addStaticNodes(const std::vector<ionet::Node>& nodes) {
		m_nodes.insert(m_nodes.end(), nodes.cbegin(), nodes.cend());
	}

	void AddStaticNodesFromPath(LocalNodeBootstrapper& bootstrapper, const std::string& path) {
		auto nodes = config::LoadPeersFromPath(path, bootstrapper.config().BlockChain.Network.Identifier);
		bootstrapper.addStaticNodes(nodes);
	}
}}
