#pragma once
#include "catapult/cache/CatapultCacheBuilder.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/observers/DemuxObserverBuilder.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/validators/DemuxValidatorBuilder.h"
#include "catapult/validators/ValidatorTypes.h"
#include "catapult/plugins.h"

namespace catapult { namespace plugins {

	/// A manager for registering plugins.
	class PluginManager {
	private:
		using HandlerHook = std::function<void (ionet::ServerPacketHandlers&, const cache::CatapultCache&)>;
		using CounterHook = std::function<void (std::vector<utils::DiagnosticCounter>&, const cache::CatapultCache&)>;
		using StatelessValidatorHook = std::function<void (validators::stateless::DemuxValidatorBuilder&)>;
		using StatefulValidatorHook = std::function<void (validators::stateful::DemuxValidatorBuilder&)>;
		using ObserverHook = std::function<void (observers::DemuxObserverBuilder&)>;

	public:
		/// Creates a new plugin manager around \a config.
		explicit PluginManager(const model::BlockChainConfiguration& config) : m_config(config)
		{}

	public:
		/// Adds support for a transaction described by \a pTransactionPlugin.
		void addTransactionSupport(std::unique_ptr<model::TransactionPlugin>&& pTransactionPlugin) {
			m_transactionRegistry.registerPlugin(std::move(pTransactionPlugin));
		}

		/// Adds support for a subcache described by \a pSubCache.
		template<typename TStorageTraits, typename TCache>
		void addCacheSupport(std::unique_ptr<TCache>&& pSubCache) {
			m_cacheBuilder.add<TStorageTraits>(std::move(pSubCache));
		}

		/// Adds a diagnostic handler \a hook.
		void addDiagnosticHandlerHook(const HandlerHook& hook) {
			m_diagnosticHandlerHooks.push_back(hook);
		}

		/// Adds a diagnostic counter \a hook.
		void addDiagnosticCounterHook(const CounterHook& hook) {
			m_diagnosticCounterHooks.push_back(hook);
		}

		/// Adds a stateless validator \a hook.
		void addStatelessValidatorHook(const StatelessValidatorHook& hook) {
			m_statelessValidatorHooks.push_back(hook);
		}

		/// Adds a stateful validator \a hook.
		void addStatefulValidatorHook(const StatefulValidatorHook& hook) {
			m_statefulValidatorHooks.push_back(hook);
		}

		/// Adds an observer \a hook.
		void addObserverHook(const ObserverHook& hook) {
			m_observerHooks.push_back(hook);
		}

		/// Adds a (transient) observer \a hook.
		void addTransientObserverHook(const ObserverHook& hook) {
			m_transientObserverHooks.push_back(hook);
		}

	public:
		/// Gets the block chain configuration.
		const model::BlockChainConfiguration& config() const {
			return m_config;
		}

		/// Gets the transaction registry.
		const model::TransactionRegistry& transactionRegistry() const {
			return m_transactionRegistry;
		}

		/// Creates a catapult cache.
		cache::CatapultCache createCache() {
			return m_cacheBuilder.build();
		}

	public:
		/// Adds all diagnostic handlers to \a handlers given \a cache.
		void addDiagnosticHandlers(ionet::ServerPacketHandlers& handlers, const cache::CatapultCache& cache) const {
			ApplyAll(handlers, m_diagnosticHandlerHooks, cache);
		}

		/// Adds all diagnostic counters to \a counters given \a cache.
		void addDiagnosticCounters(std::vector<utils::DiagnosticCounter>& counters, const cache::CatapultCache& cache) const {
			ApplyAll(counters, m_diagnosticCounterHooks, cache);
		}

	private:
		template<typename TBuilder, typename THooks, typename... TArgs>
		static void ApplyAll(TBuilder& builder, const THooks& hooks, TArgs&&... args) {
			for (const auto& hook : hooks)
				hook(builder, std::forward<TArgs>(args)...);
		}

		template<typename TBuilder, typename THooks>
		static auto Build(const THooks& hooks) {
			TBuilder builder;
			ApplyAll(builder, hooks);
			return builder.build();
		}

	public:
		/// Creates a stateless validator.
		auto createStatelessValidator() const {
			return Build<validators::stateless::DemuxValidatorBuilder>(m_statelessValidatorHooks);
		}

		/// Creates a stateful validator.
		auto createStatefulValidator() const {
			return Build<validators::stateful::DemuxValidatorBuilder>(m_statefulValidatorHooks);
		}

		/// Creates an observer.
		auto createObserver() const {
			observers::DemuxObserverBuilder builder;
			ApplyAll(builder, m_observerHooks);
			ApplyAll(builder, m_transientObserverHooks);
			return builder.build();
		}

		/// Creates an observer that only observes permanent state changes.
		auto createPermanentObserver() const {
			return Build<observers::DemuxObserverBuilder>(m_observerHooks);
		}

	private:
		model::BlockChainConfiguration m_config;
		model::TransactionRegistry m_transactionRegistry;
		cache::CatapultCacheBuilder m_cacheBuilder;

		std::vector<HandlerHook> m_diagnosticHandlerHooks;
		std::vector<CounterHook> m_diagnosticCounterHooks;
		std::vector<StatelessValidatorHook> m_statelessValidatorHooks;
		std::vector<StatefulValidatorHook> m_statefulValidatorHooks;
		std::vector<ObserverHook> m_observerHooks;
		std::vector<ObserverHook> m_transientObserverHooks;
	};
}}

/// Entry point for registering a dynamic module with \a manager.
extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager);
