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

#include "ProcessBootstrapper.h"
#include "PluginUtils.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/utils/Logging.h"
#include <boost/exception_ptr.hpp>

namespace catapult { namespace extensions {

#ifdef STRICT_SYMBOL_VISIBILITY
	template<typename T>
	void ForceSymbolInjection() {
		CATAPULT_LOG(debug) << "forcibly injecting symbol: " << typeid(T).name () << " => " << typeid(T).hash_code();
	}
#endif

	ProcessBootstrapper::ProcessBootstrapper(
			const config::CatapultConfiguration& config,
			const std::string& resourcesPath,
			ProcessDisposition disposition,
			const std::string& servicePoolName)
			: m_config(std::move(config))
			, m_resourcesPath(resourcesPath)
			, m_disposition(disposition)
			, m_pMultiServicePool(std::make_unique<thread::MultiServicePool>(
					servicePoolName,
					thread::MultiServicePool::DefaultPoolConcurrency(),
					m_config.Node.EnableSingleThreadPool
							? thread::MultiServicePool::IsolatedPoolMode::Disabled
							: thread::MultiServicePool::IsolatedPoolMode::Enabled))
			, m_subscriptionManager(config)
			, m_pluginManager(m_config.BlockChain, CreateStorageConfiguration(config), m_config.User, m_config.Inflation) {
#ifdef STRICT_SYMBOL_VISIBILITY
			// need to forcibly inject typeinfos into containing exe so that they are properly resolved across modules
			ForceSymbolInjection<model::EmbeddedTransactionPlugin>();
#endif
	}

	const config::CatapultConfiguration& ProcessBootstrapper::config() const {
		return m_config;
	}

	const std::string& ProcessBootstrapper::resourcesPath() const {
		return m_resourcesPath;
	}

	ProcessDisposition ProcessBootstrapper::disposition() const {
		return m_disposition;
	}

	const std::vector<ionet::Node>& ProcessBootstrapper::staticNodes() const {
		return m_nodes;
	}

	thread::MultiServicePool& ProcessBootstrapper::pool() {
		return *m_pMultiServicePool;
	}

	ExtensionManager& ProcessBootstrapper::extensionManager() {
		return m_extensionManager;
	}

	subscribers::SubscriptionManager& ProcessBootstrapper::subscriptionManager() {
		return m_subscriptionManager;
	}

	plugins::PluginManager& ProcessBootstrapper::pluginManager() {
		return m_pluginManager;
	}

	namespace {
		void LoadExtension(const plugins::PluginModule& module, ProcessBootstrapper& bootstrapper) {
			auto registerExtension = module.symbol<decltype(::RegisterExtension)*>("RegisterExtension");

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

	void ProcessBootstrapper::loadExtensions() {
		for (const auto& extension : m_config.Extensions.Names) {
			auto scope = plugins::PluginModule::Scope::Local;

#ifdef STRICT_SYMBOL_VISIBILITY
			// any extensions that provide additional plugin models need to be imported globally so that their symbols can
			// be used to resolve usages in their plugins
			if ("extension.mongo" == extension)
				scope = plugins::PluginModule::Scope::Global;
#endif

			m_extensionModules.emplace_back(m_config.User.PluginsDirectory, extension, scope);

			CATAPULT_LOG(info) << "registering dynamic extension " << extension;
			LoadExtension(m_extensionModules.back(), *this);
		}
	}

	void ProcessBootstrapper::addStaticNodes(const std::vector<ionet::Node>& nodes) {
		m_nodes.insert(m_nodes.end(), nodes.cbegin(), nodes.cend());
	}

	void AddStaticNodesFromPath(ProcessBootstrapper& bootstrapper, const std::string& path) {
		auto nodes = config::LoadPeersFromPath(path, bootstrapper.config().BlockChain.Network.Identifier);
		bootstrapper.addStaticNodes(nodes);
	}
}}
