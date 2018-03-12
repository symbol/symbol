#pragma once
#include "catapult/cache/CatapultCacheBuilder.h"
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

	/// A manager for registering plugins.
	class PluginManager {
	private:
		using HandlerHook = consumer<ionet::ServerPacketHandlers&, const cache::CatapultCache&>;
		using CounterHook = consumer<std::vector<utils::DiagnosticCounter>&, const cache::CatapultCache&>;
		using StatelessValidatorHook = consumer<validators::stateless::DemuxValidatorBuilder&>;
		using StatefulValidatorHook = consumer<validators::stateful::DemuxValidatorBuilder&>;
		using ObserverHook = consumer<observers::DemuxObserverBuilder&>;

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

		template<typename TBuilder, typename THooks, typename... TArgs>
		static auto Build(const THooks& hooks, TArgs&&... args) {
			TBuilder builder;
			ApplyAll(builder, hooks);
			return builder.build(std::forward<TArgs>(args)...);
		}

	public:
		/// Creates a stateless validator that ignores suppressed failures according to \a isSuppressedFailure.
		auto createStatelessValidator(const validators::ValidationResultPredicate& isSuppressedFailure) const {
			return Build<validators::stateless::DemuxValidatorBuilder>(m_statelessValidatorHooks, isSuppressedFailure);
		}

		/// Creates a stateless validator with no suppressed failures.
		auto createStatelessValidator() const {
			return createStatelessValidator([](auto) { return false; });
		}

		/// Creates a stateful validator that ignores suppressed failures according to \a isSuppressedFailure.
		auto createStatefulValidator(const validators::ValidationResultPredicate& isSuppressedFailure) const {
			return Build<validators::stateful::DemuxValidatorBuilder>(m_statefulValidatorHooks, isSuppressedFailure);
		}

		/// Creates a stateful validator with no suppressed failures.
		auto createStatefulValidator() const {
			return createStatefulValidator([](auto) { return false; });
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

		/// Creates a notification publisher for the specified \a mode.
		auto createNotificationPublisher(model::PublicationMode mode = model::PublicationMode::All) const {
			return model::CreateNotificationPublisher(m_transactionRegistry, mode);
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
