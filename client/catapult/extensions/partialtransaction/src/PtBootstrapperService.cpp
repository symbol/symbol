#include "PtBootstrapperService.h"
#include "catapult/cache/MemoryPtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/Results.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"

namespace catapult { namespace partialtransaction {

	namespace {
		using PtCache = cache::MemoryPtCacheProxy;

		constexpr auto Cache_Service_Name = "pt.cache";
		constexpr auto Hooks_Service_Name = "pt.hooks";

		class PtBootstrapperServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit PtBootstrapperServiceRegistrar(const PtCacheSupplier& ptCacheSupplier) : m_ptCacheSupplier(ptCacheSupplier)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "PtBootstrapper", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<PtCache>(Cache_Service_Name, "PT CACHE", [](const auto& cache) {
					return cache.view().size();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// register services
				auto pCache = utils::UniqueToShared(m_ptCacheSupplier());
				locator.registerRootedService(Cache_Service_Name, pCache);
				locator.registerRootedService(Hooks_Service_Name, std::make_shared<PtServerHooks>());

				// register hooks
				auto& ptCache = *pCache;
				auto& transactionStatusSubscriber = state.transactionStatusSubscriber();
				auto timeSupplier = state.timeSupplier();
				state.hooks().addTransactionsChangeHandler([&ptCache, &transactionStatusSubscriber, timeSupplier](const auto& changeInfo) {
					// 1. remove all confirmed transactions from pt cache
					auto modifier = ptCache.modifier();
					for (const auto* pHash : changeInfo.AddedTransactionHashes)
						modifier.remove(*pHash);

					// 2. prune the pt cache
					auto pruneStatus = utils::to_underlying_type(extensions::Failure_Extension_Partial_Transaction_Cache_Prune);
					auto prunedInfos = modifier.prune(timeSupplier());
					for (const auto& prunedInfo : prunedInfos)
						transactionStatusSubscriber.notifyStatus(*prunedInfo.pEntity, prunedInfo.EntityHash, pruneStatus);
				});

				state.hooks().addTransactionEventHandler([&ptCache, &transactionStatusSubscriber](const auto& eventData) {
					if (!HasFlag(extensions::TransactionEvent::Dependency_Removed, eventData.Event))
						return;

					// if a transaction's dependency (e.g. partial aggregate bond) was removed, remove the transaction from the pt cache
					auto removedInfo = ptCache.modifier().remove(eventData.TransactionHash);
					if (removedInfo) {
						auto status = utils::to_underlying_type(extensions::Failure_Extension_Partial_Transaction_Dependency_Removed);
						transactionStatusSubscriber.notifyStatus(*removedInfo.pEntity, removedInfo.EntityHash, status);
					}
				});
			}

		private:
			PtCacheSupplier m_ptCacheSupplier;
		};
	}

	DECLARE_SERVICE_REGISTRAR(PtBootstrapper)(const PtCacheSupplier& ptCacheSupplier) {
		return std::make_unique<PtBootstrapperServiceRegistrar>(ptCacheSupplier);
	}

	PtCache& GetMemoryPtCache(const extensions::ServiceLocator& locator) {
		return *locator.service<PtCache>(Cache_Service_Name);
	}

	PtServerHooks& GetPtServerHooks(const extensions::ServiceLocator& locator) {
		return *locator.service<PtServerHooks>(Hooks_Service_Name);
	}
}}
