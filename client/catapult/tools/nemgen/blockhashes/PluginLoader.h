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
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionPlugin.h"
#include <memory>

namespace catapult {
	namespace config { class CatapultConfiguration; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace tools { namespace nemgen {

	/// Possible cache database cleanup modes.
	enum class CacheDatabaseCleanupMode {
		/// Perform no cleanup.
		None,

		/// Purge after execution.
		Purge
	};

	/// Loads plugins into a plugin manager.
	class PluginLoader {
	public:
		/// Creates a loader around \a config with specified database cleanup mode (\a databaseCleanupMode).
		PluginLoader(const config::CatapultConfiguration& config, CacheDatabaseCleanupMode databaseCleanupMode);

		/// Destroys the loader.
		~PluginLoader();

	public:
		/// Gets the plugin manager.
		plugins::PluginManager& manager();

		/// Gets the transaction registry.
		const model::TransactionRegistry& transactionRegistry() const;

		/// Creates a notification publisher.
		std::unique_ptr<const model::NotificationPublisher> createNotificationPublisher() const;

	public:
		/// Loads all configured plugins.
		void loadAll();

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}}
