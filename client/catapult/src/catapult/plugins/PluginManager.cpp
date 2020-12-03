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

#include "PluginManager.h"
#include <filesystem>

namespace catapult { namespace plugins {

	PluginManager::PluginManager(
			const model::BlockChainConfiguration& config,
			const StorageConfiguration& storageConfig,
			const config::UserConfiguration& userConfig,
			const config::InflationConfiguration& inflationConfig)
			: m_config(config)
			, m_storageConfig(storageConfig)
			, m_userConfig(userConfig)
			, m_inflationConfig(inflationConfig)
	{}

	// region config

	const model::BlockChainConfiguration& PluginManager::config() const {
		return m_config;
	}

	const StorageConfiguration& PluginManager::storageConfig() const {
		return m_storageConfig;
	}

	const config::UserConfiguration& PluginManager::userConfig() const {
		return m_userConfig;
	}

	const config::InflationConfiguration& PluginManager::inflationConfig() const {
		return m_inflationConfig;
	}

	cache::CacheConfiguration PluginManager::cacheConfig(const std::string& name) const {
		if (!m_storageConfig.PreferCacheDatabase)
			return cache::CacheConfiguration();

		return cache::CacheConfiguration(
				(std::filesystem::path(m_storageConfig.CacheDatabaseDirectory) / name).generic_string(),
				m_storageConfig.CacheDatabaseConfig,
				m_config.EnableVerifiableState ? cache::PatriciaTreeStorageMode::Enabled : cache::PatriciaTreeStorageMode::Disabled);
	}

	// endregion

	// region transactions

	void PluginManager::addTransactionSupport(std::unique_ptr<model::TransactionPlugin>&& pTransactionPlugin) {
		m_transactionRegistry.registerPlugin(std::move(pTransactionPlugin));
	}

	const model::TransactionRegistry& PluginManager::transactionRegistry() const {
		return m_transactionRegistry;
	}

	// endregion

	// region cache

	void PluginManager::addCacheSupport(std::unique_ptr<cache::SubCachePlugin>&& pSubCachePlugin) {
		m_cacheBuilder.add(std::move(pSubCachePlugin));
	}

	cache::CatapultCache PluginManager::createCache() {
		return m_cacheBuilder.build();
	}

	// endregion

	namespace {
		template<typename TBuilder, typename THooks, typename... TArgs>
		static void ApplyAll(TBuilder& builder, const THooks& hooks, TArgs&&... args) {
			for (const auto& hook : hooks)
				hook(builder, std::forward<TArgs>(args)...);
		}

		template<typename TBuilder, typename THooks, typename... TArgs>
		static auto Build(const THooks& hooks, TArgs&&... args) {
			TBuilder builder;
			ApplyAll(builder, hooks);
			return builder.build(std::forward<TArgs>(args)...);
		}
	}

	// region handlers

	void PluginManager::addHandlerHook(const HandlerHook& hook) {
		m_nonDiagnosticHandlerHooks.push_back(hook);
	}

	void PluginManager::addHandlers(ionet::ServerPacketHandlers& handlers, const cache::CatapultCache& cache) const {
		ApplyAll(handlers, m_nonDiagnosticHandlerHooks, cache);
	}

	// endregion

	// region diagnostics

	void PluginManager::addDiagnosticHandlerHook(const HandlerHook& hook) {
		m_diagnosticHandlerHooks.push_back(hook);
	}

	void PluginManager::addDiagnosticCounterHook(const CounterHook& hook) {
		m_diagnosticCounterHooks.push_back(hook);
	}

	void PluginManager::addDiagnosticHandlers(ionet::ServerPacketHandlers& handlers, const cache::CatapultCache& cache) const {
		ApplyAll(handlers, m_diagnosticHandlerHooks, cache);
	}

	void PluginManager::addDiagnosticCounters(std::vector<utils::DiagnosticCounter>& counters, const cache::CatapultCache& cache) const {
		ApplyAll(counters, m_diagnosticCounterHooks, cache);
	}

	// endregion

	// region validators

	void PluginManager::addStatelessValidatorHook(const StatelessValidatorHook& hook) {
		m_statelessValidatorHooks.push_back(hook);
	}

	void PluginManager::addStatefulValidatorHook(const StatefulValidatorHook& hook) {
		m_statefulValidatorHooks.push_back(hook);
	}

	PluginManager::StatelessValidatorPointer PluginManager::createStatelessValidator(
			const validators::ValidationResultPredicate& isSuppressedFailure) const {
		return Build<validators::stateless::DemuxValidatorBuilder>(m_statelessValidatorHooks, isSuppressedFailure);
	}

	PluginManager::StatelessValidatorPointer PluginManager::createStatelessValidator() const {
		return createStatelessValidator([](auto) { return false; });
	}

	PluginManager::StatefulValidatorPointer PluginManager::createStatefulValidator(
			const validators::ValidationResultPredicate& isSuppressedFailure) const {
		return Build<validators::stateful::DemuxValidatorBuilder>(m_statefulValidatorHooks, isSuppressedFailure);
	}

	PluginManager::StatefulValidatorPointer PluginManager::createStatefulValidator() const {
		return createStatefulValidator([](auto) { return false; });
	}

	// endregion

	// region observers

	void PluginManager::addObserverHook(const ObserverHook& hook) {
		m_observerHooks.push_back(hook);
	}

	void PluginManager::addTransientObserverHook(const ObserverHook& hook) {
		m_transientObserverHooks.push_back(hook);
	}

	PluginManager::ObserverPointer PluginManager::createObserver() const {
		observers::DemuxObserverBuilder builder;
		ApplyAll(builder, m_observerHooks);
		ApplyAll(builder, m_transientObserverHooks);
		return builder.build();
	}

	PluginManager::ObserverPointer PluginManager::createPermanentObserver() const {
		return Build<observers::DemuxObserverBuilder>(m_observerHooks);
	}

	// endregion

	// region resolvers

	void PluginManager::addMosaicResolver(const MosaicResolver& resolver) {
		m_mosaicResolvers.push_back(resolver);
	}

	void PluginManager::addAddressResolver(const AddressResolver& resolver) {
		m_addressResolvers.push_back(resolver);
	}

	namespace {
		template<typename TResolved, typename TResolvers>
		auto BuildResolver(const TResolvers& resolvers) {
			return [resolvers](const auto& cache, const auto& unresolved) {
				TResolved resolved;
				for (const auto& resolver : resolvers) {
					if (resolver(cache, unresolved, resolved))
						return resolved;
				}

				return model::ResolverContext().resolve(unresolved);
			};
		}
	}

	model::ResolverContext PluginManager::createResolverContext(const cache::ReadOnlyCatapultCache& cache) const {
		auto mosaicResolver = BuildResolver<MosaicId>(m_mosaicResolvers);
		auto addressResolver = BuildResolver<Address>(m_addressResolvers);

		auto bindResolverToCache = [&cache](const auto& resolver) {
			return [&cache, resolver](const auto& unresolved) { return resolver(cache, unresolved); };
		};

		return model::ResolverContext(bindResolverToCache(mosaicResolver), bindResolverToCache(addressResolver));
	}

	// endregion

	// region publisher

	PluginManager::PublisherPointer PluginManager::createNotificationPublisher(model::PublicationMode mode) const {
		return model::CreateNotificationPublisher(m_transactionRegistry, model::GetUnresolvedCurrencyMosaicId(m_config), mode);
	}

	// endregion
}}
