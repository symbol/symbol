/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/cache/CacheConfiguration.h"
#include "catapult/cache/CatapultCacheBuilder.h"
#include "catapult/config/InflationConfiguration.h"
#include "catapult/config/UserConfiguration.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/observers/DemuxObserverBuilder.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/validators/DemuxValidatorBuilder.h"
#include "catapult/validators/ValidatorTypes.h"
#include "catapult/plugins.h"

namespace catapult { namespace plugins {

	/// Additional storage configuration.
	struct StorageConfiguration {
	public:
		/// Creates default configuration.
		StorageConfiguration()
				: PreferCacheDatabase(false)
				, CacheDatabaseConfig() // default initialize
		{}

	public:
		/// Prefer using a database for cache storage.
		bool PreferCacheDatabase;

		/// Base directory to use for storing cache database.
		std::string CacheDatabaseDirectory;

		/// Cache database configuration.
		config::NodeConfiguration::CacheDatabaseSubConfiguration CacheDatabaseConfig;
	};

	/// Manager for registering plugins.
	class PLUGIN_API_DEPENDENCY PluginManager {
	private:
		using HandlerHook = consumer<ionet::ServerPacketHandlers&, const cache::CatapultCache&>;
		using CounterHook = consumer<std::vector<utils::DiagnosticCounter>&, const cache::CatapultCache&>;
		using StatelessValidatorHook = consumer<validators::stateless::DemuxValidatorBuilder&>;
		using StatefulValidatorHook = consumer<validators::stateful::DemuxValidatorBuilder&>;
		using ObserverHook = consumer<observers::DemuxObserverBuilder&>;

		using StatelessValidatorPointer = std::unique_ptr<const validators::stateless::AggregateNotificationValidator>;
		using StatefulValidatorPointer = std::unique_ptr<const validators::stateful::AggregateNotificationValidator>;
		using ObserverPointer = observers::AggregateNotificationObserverPointerT<model::Notification>;

		template<typename TUnresolved, typename TResolved>
		using Resolver = predicate<const cache::ReadOnlyCatapultCache&, const TUnresolved&, TResolved&>;
		using MosaicResolver = Resolver<UnresolvedMosaicId, MosaicId>;
		using AddressResolver = Resolver<UnresolvedAddress, Address>;

		template<typename TUnresolved, typename TResolved>
		using AggregateResolver = std::function<TResolved (const cache::ReadOnlyCatapultCache&, const TUnresolved&)>;
		using AggregateMosaicResolver = AggregateResolver<UnresolvedMosaicId, MosaicId>;
		using AggregateAddressResolver = AggregateResolver<UnresolvedAddress, Address>;

		using PublisherPointer = std::unique_ptr<const model::NotificationPublisher>;

	public:
		/// Creates a new plugin manager around \a config, \a storageConfig \a userConfig and \a inflationConfig.
		PluginManager(
				const model::BlockChainConfiguration& config,
				const StorageConfiguration& storageConfig,
				const config::UserConfiguration& userConfig,
				const config::InflationConfiguration& inflationConfig);

	public:
		// region config

		/// Gets the block chain configuration.
		const model::BlockChainConfiguration& config() const;

		/// Gets the storage configuration.
		const StorageConfiguration& storageConfig() const;

		/// Gets the user configuration.
		const config::UserConfiguration& userConfig() const;

		/// Gets the inflation configuration.
		const config::InflationConfiguration& inflationConfig() const;

		/// Gets the cache configuration for cache with \a name.
		cache::CacheConfiguration cacheConfig(const std::string& name) const;

		// endregion

		// region transactions

		/// Adds support for a transaction described by \a pTransactionPlugin.
		void addTransactionSupport(std::unique_ptr<model::TransactionPlugin>&& pTransactionPlugin);

		/// Gets the transaction registry.
		const model::TransactionRegistry& transactionRegistry() const;

		// endregion

		// region cache

		/// Adds support for a sub cache described by \a pSubCache.
		template<typename TStorageTraits, typename TCache>
		void addCacheSupport(std::unique_ptr<TCache>&& pSubCache) {
			m_cacheBuilder.add<TStorageTraits>(std::move(pSubCache));
		}

		/// Adds support for a sub cache registered by \a pSubCachePlugin.
		void addCacheSupport(std::unique_ptr<cache::SubCachePlugin>&& pSubCachePlugin);

		/// Creates a catapult cache.
		cache::CatapultCache createCache();

		// endregion

		// region handlers

		/// Adds a (non-diagnostic) handler \a hook.
		void addHandlerHook(const HandlerHook& hook);

		/// Adds all (non-diagnostic) handlers to \a handlers given \a cache.
		void addHandlers(ionet::ServerPacketHandlers& handlers, const cache::CatapultCache& cache) const;

		// endregion

		// region diagnostics

		/// Adds a diagnostic handler \a hook.
		void addDiagnosticHandlerHook(const HandlerHook& hook);

		/// Adds a diagnostic counter \a hook.
		void addDiagnosticCounterHook(const CounterHook& hook);

		/// Adds all diagnostic handlers to \a handlers given \a cache.
		void addDiagnosticHandlers(ionet::ServerPacketHandlers& handlers, const cache::CatapultCache& cache) const;

		/// Adds all diagnostic counters to \a counters given \a cache.
		void addDiagnosticCounters(std::vector<utils::DiagnosticCounter>& counters, const cache::CatapultCache& cache) const;

		// endregion

		// region validators

		/// Adds a stateless validator \a hook.
		void addStatelessValidatorHook(const StatelessValidatorHook& hook);

		/// Adds a stateful validator \a hook.
		void addStatefulValidatorHook(const StatefulValidatorHook& hook);

		/// Creates a stateless validator that ignores suppressed failures according to \a isSuppressedFailure.
		StatelessValidatorPointer createStatelessValidator(const validators::ValidationResultPredicate& isSuppressedFailure) const;

		/// Creates a stateless validator with no suppressed failures.
		StatelessValidatorPointer createStatelessValidator() const;

		/// Creates a stateful validator that ignores suppressed failures according to \a isSuppressedFailure.
		StatefulValidatorPointer createStatefulValidator(const validators::ValidationResultPredicate& isSuppressedFailure) const;

		/// Creates a stateful validator with no suppressed failures.
		StatefulValidatorPointer createStatefulValidator() const;

		// endregion

		// region observers

		/// Adds an observer \a hook.
		void addObserverHook(const ObserverHook& hook);

		/// Adds a (transient) observer \a hook.
		void addTransientObserverHook(const ObserverHook& hook);

		/// Creates an observer.
		ObserverPointer createObserver() const;

		/// Creates an observer that only observes permanent state changes.
		ObserverPointer createPermanentObserver() const;

		// endregion

		// region resolvers

		/// Adds a mosaic \a resolver.
		void addMosaicResolver(const MosaicResolver& resolver);

		/// Adds an address \a resolver.
		void addAddressResolver(const AddressResolver& resolver);

		/// Creates a resolver context given \a cache.
		model::ResolverContext createResolverContext(const cache::ReadOnlyCatapultCache& cache) const;

		// endregion

		// region publisher

		/// Creates a notification publisher for the specified \a mode.
		PublisherPointer createNotificationPublisher(model::PublicationMode mode = model::PublicationMode::All) const;

		// endregion

	private:
		model::BlockChainConfiguration m_config;
		StorageConfiguration m_storageConfig;
		config::UserConfiguration m_userConfig;
		config::InflationConfiguration m_inflationConfig;
		model::TransactionRegistry m_transactionRegistry;
		cache::CatapultCacheBuilder m_cacheBuilder;

		std::vector<HandlerHook> m_nonDiagnosticHandlerHooks;
		std::vector<HandlerHook> m_diagnosticHandlerHooks;
		std::vector<CounterHook> m_diagnosticCounterHooks;
		std::vector<StatelessValidatorHook> m_statelessValidatorHooks;
		std::vector<StatefulValidatorHook> m_statefulValidatorHooks;
		std::vector<ObserverHook> m_observerHooks;
		std::vector<ObserverHook> m_transientObserverHooks;

		std::vector<MosaicResolver> m_mosaicResolvers;
		std::vector<AddressResolver> m_addressResolvers;
	};
}}

/// Entry point for registering a dynamic module with \a manager.
extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager);
