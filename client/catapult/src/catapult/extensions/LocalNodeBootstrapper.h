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

#pragma once
#include "ExtensionManager.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/plugins/PluginModule.h"
#include "catapult/subscribers/SubscriptionManager.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/plugins.h"

namespace catapult { namespace extensions {

	/// A local node bootstrapper.
	class LocalNodeBootstrapper {
	public:
		/// Creates a local node bootstrapper around \a config and \a servicePoolName given the specified
		/// resources path (\a resourcesPath).
		LocalNodeBootstrapper(
				const config::LocalNodeConfiguration& config,
				const std::string& resourcesPath,
				const std::string& servicePoolName);

	public:
		/// Gets the configuration.
		const config::LocalNodeConfiguration& config() const;

		/// Gets the resources path.
		const std::string& resourcesPath() const;

		/// Gets the static (pretrusted) nodes.
		const std::vector<ionet::Node>& staticNodes() const;

	public:
		/// Gets the multiservice pool.
		thread::MultiServicePool& pool();

		/// Gets the extension manager.
		ExtensionManager& extensionManager();

		/// Gets the subscription manager.
		subscribers::SubscriptionManager& subscriptionManager();

		/// Gets the plugin manager.
		plugins::PluginManager& pluginManager();

	public:
		/// Loads all configured extensions.
		void loadExtensions();

		/// Adds static \a nodes to the bootstrapper.
		void addStaticNodes(const std::vector<ionet::Node>& nodes);

	private:
		config::LocalNodeConfiguration m_config;
		std::string m_resourcesPath;
		std::unique_ptr<thread::MultiServicePool> m_pMultiServicePool;
		std::vector<plugins::PluginModule> m_extensionModules;

		ExtensionManager m_extensionManager;
		subscribers::SubscriptionManager m_subscriptionManager;
		plugins::PluginManager m_pluginManager;
		std::vector<ionet::Node> m_nodes;
	};

	/// Adds static nodes from \a path to \a bootstrapper.
	void AddStaticNodesFromPath(LocalNodeBootstrapper& bootstrapper, const std::string& path);
}}

/// Entry point for registering a dynamic extension module with \a bootstrapper.
extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper);
