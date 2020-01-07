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
#include "ServerHooks.h"
#include "ServiceState.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/net/PacketIoPickerContainer.h"
#include "catapult/thread/Task.h"

namespace catapult {
	namespace cache {
		class CatapultCache;
		class ReadWriteUtCache;
		class UtCache;
	}
	namespace extensions { class LocalNodeChainScore; }
	namespace io { class BlockStorageCache; }
	namespace ionet { class NodeContainer; }
	namespace plugins { class PluginManager; }
	namespace subscribers {
		class NodeSubscriber;
		class StateChangeSubscriber;
		class TransactionStatusSubscriber;
	}
	namespace thread { class MultiServicePool; }
	namespace utils { class DiagnosticCounter; }
}

namespace catapult { namespace extensions {

	/// State that is used as part of service registration.
	class ServiceState {
	public:
		/// Creates service state around \a config, \a nodes, \a cache, \a storage, \a score, \a utCache, \a timeSupplier
		/// \a transactionStatusSubscriber, \a stateChangeSubscriber, \a nodeSubscriber, \a counters, \a pluginManager and \a pool.
		ServiceState(
				const config::CatapultConfiguration& config,
				ionet::NodeContainer& nodes,
				cache::CatapultCache& cache,
				io::BlockStorageCache& storage,
				LocalNodeChainScore& score,
				cache::ReadWriteUtCache& utCache,
				const supplier<Timestamp>& timeSupplier,
				subscribers::TransactionStatusSubscriber& transactionStatusSubscriber,
				subscribers::StateChangeSubscriber& stateChangeSubscriber,
				subscribers::NodeSubscriber& nodeSubscriber,
				const std::vector<utils::DiagnosticCounter>& counters,
				const plugins::PluginManager& pluginManager,
				thread::MultiServicePool& pool)
				: m_config(config)
				, m_nodes(nodes)
				, m_cache(cache)
				, m_storage(storage)
				, m_score(score)
				, m_utCache(utCache)
				, m_timeSupplier(timeSupplier)
				, m_transactionStatusSubscriber(transactionStatusSubscriber)
				, m_stateChangeSubscriber(stateChangeSubscriber)
				, m_nodeSubscriber(nodeSubscriber)
				, m_counters(counters)
				, m_pluginManager(pluginManager)
				, m_pool(pool)
				, m_packetHandlers(m_config.Node.MaxPacketDataSize.bytes32())
		{}

	public:
		/// Gets the config.
		const auto& config() const {
			return m_config;
		}

		/// Gets the nodes.
		auto& nodes() const {
			return m_nodes;
		}

		/// Gets the cache.
		auto& cache() const {
			return m_cache;
		}

		/// Gets the storage.
		auto& storage() const {
			return m_storage;
		}

		/// Gets the score.
		auto& score() const {
			return m_score;
		}

		/// Gets the unconfirmed transactions cache.
		const cache::ReadWriteUtCache& utCache() const {
			return m_utCache;
		}

		/// Gets the unconfirmed transactions cache.
		cache::UtCache& utCache() {
			return m_utCache;
		}

		/// Gets the time supplier.
		auto timeSupplier() const {
			return m_timeSupplier;
		}

		/// Gets the transaction status subscriber.
		auto& transactionStatusSubscriber() const {
			return m_transactionStatusSubscriber;
		}

		/// Gets the state change subscriber.
		auto& stateChangeSubscriber() const {
			return m_stateChangeSubscriber;
		}

		/// Gets the node subscriber.
		auto& nodeSubscriber() const {
			return m_nodeSubscriber;
		}

		/// Gets the (basic) counters.
		/// \note These counters are node counters and do not include counters registered via ServiceLocator.
		const auto& counters() const {
			return m_counters;
		}

		/// Gets the plugin manager.
		const auto& pluginManager() const {
			return m_pluginManager;
		}

		/// Gets the multiservice pool.
		auto& pool() {
			return m_pool;
		}

		/// Gets the tasks.
		auto& tasks() {
			return m_tasks;
		}

		/// Gets the packet handlers.
		auto& packetHandlers() {
			return m_packetHandlers;
		}

		/// Gets the server hooks.
		const auto& hooks() const {
			return m_hooks;
		}

		/// Gets the server hooks.
		auto& hooks() {
			return m_hooks;
		}

		/// Gets the packet io pickers.
		auto& packetIoPickers() {
			return m_packetIoPickers;
		}

	private:
		// references
		const config::CatapultConfiguration& m_config;
		ionet::NodeContainer& m_nodes;
		cache::CatapultCache& m_cache;
		io::BlockStorageCache& m_storage;
		LocalNodeChainScore& m_score;
		cache::ReadWriteUtCache& m_utCache;
		supplier<Timestamp> m_timeSupplier;

		subscribers::TransactionStatusSubscriber& m_transactionStatusSubscriber;
		subscribers::StateChangeSubscriber& m_stateChangeSubscriber;
		subscribers::NodeSubscriber& m_nodeSubscriber;

		const std::vector<utils::DiagnosticCounter>& m_counters;
		const plugins::PluginManager& m_pluginManager;
		thread::MultiServicePool& m_pool;

		// owned
		std::vector<thread::Task> m_tasks;
		ionet::ServerPacketHandlers m_packetHandlers;
		ServerHooks m_hooks;
		net::PacketIoPickerContainer m_packetIoPickers;
	};
}}
