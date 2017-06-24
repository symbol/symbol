#include "UnconfirmedTransactionsUpdater.h"
#include "ProcessingNotificationSubscriber.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/UtCache.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace chain {

	namespace {
		// this wrapper is needed because CatapultCacheDetachedDelta / LockableCacheDelta is not assignable
		// due to a const& member variable
		class DetachedDeltaWrapper {
		public:
			explicit DetachedDeltaWrapper(cache::CatapultCacheDetachedDelta&& detachedDelta)
					: m_detachedDelta(std::move(detachedDelta))
			{}

		public:
			auto lock() {
				return m_detachedDelta.lock();
			}

		private:
			cache::CatapultCacheDetachedDelta m_detachedDelta;
		};

		struct ApplyState {
			constexpr ApplyState(
					cache::UtCacheModifierProxy& modifier,
					cache::CatapultCacheDelta& unconfirmedCatapultCache)
					: Modifier(modifier)
					, UnconfirmedCatapultCache(unconfirmedCatapultCache)
			{}

			cache::UtCacheModifierProxy& Modifier;
			cache::CatapultCacheDelta& UnconfirmedCatapultCache;
		};
	}

	class UnconfirmedTransactionsUpdater::Impl final {
	public:
		Impl(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				const ExecutionConfiguration& config,
				const TimeProvider& timeProvider)
				: m_transactionsCache(transactionsCache)
				, m_confirmedCatapultCache(confirmedCatapultCache)
				, m_config(config)
				, m_timeProvider(timeProvider) {
			rebaseAndLock();
		}

	public:
		void update(std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
			// 1. lock the UT cache and lock the unconfirmed copy
			auto modifier = m_transactionsCache.modifier();
			auto pUnconfirmedCatapultCache = m_pLockableUnconfirmedCatapultCache->lock();
			if (!pUnconfirmedCatapultCache) {
				// if there is no unconfirmed cache state, it means that a block update is forthcoming
				// just add all to the cache and they will be validated later
				addAll(modifier, std::move(unconfirmedTransactionInfos));
				return;
			}

			auto applyState = ApplyState(modifier, *pUnconfirmedCatapultCache);
			apply(applyState, std::move(unconfirmedTransactionInfos));
		}

		void update(
				const utils::HashPointerSet& confirmedTransactionHashes,
				std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
			CATAPULT_LOG(debug)
					<< "confirmed " << confirmedTransactionHashes.size() << " transactions, "
					<< "reverted " << unconfirmedTransactionInfos.size() << " transactions";

			// 1. lock the catapult cache and rebase the unconfirmed catapult cache
			auto pUnconfirmedCatapultCache = rebaseAndLock();

			// 2. lock and clear the UT cache
			auto modifier = m_transactionsCache.modifier();
			auto originalTransactionInfos = modifier.removeAll();

			// 3. add back reverted txes
			auto applyState = ApplyState(modifier, *pUnconfirmedCatapultCache);
			apply(applyState, std::move(unconfirmedTransactionInfos));

			// 4. add back original txes that have not been confirmed
			apply(applyState, std::move(originalTransactionInfos), [&confirmedTransactionHashes](const auto& info) {
				return confirmedTransactionHashes.cend() == confirmedTransactionHashes.find(&info.EntityHash);
			});
		}

	private:
		std::unique_ptr<cache::CatapultCacheDelta> rebaseAndLock() {
			auto detachableDelta = m_confirmedCatapultCache.createDetachableDelta();
			m_cacheHeight = detachableDelta.height();
			m_pLockableUnconfirmedCatapultCache = std::make_unique<DetachedDeltaWrapper>(detachableDelta.detach());
			return m_pLockableUnconfirmedCatapultCache->lock();
		}

		void apply(const ApplyState& applyState, std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
			apply(applyState, std::move(unconfirmedTransactionInfos), [](const auto&) { return true; });
		}

		void apply(
				const ApplyState& applyState,
				std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos,
				const std::function<bool (const model::TransactionInfo&)>& filter) {
			auto currentTime = m_timeProvider();

			auto readOnlyCache = applyState.UnconfirmedCatapultCache.toReadOnly();

			// note that the validator and observer context height is one larger than the chain height
			// since the validation and observation has to be for the *next* block
			auto effectiveHeight = m_cacheHeight + Height(1);
			auto validatorContext = validators::ValidatorContext(effectiveHeight, currentTime, m_config.Network, readOnlyCache);

			// note that the "real" state is currently only required by block observers, so a dummy state can be used
			auto& cache = applyState.UnconfirmedCatapultCache;
			state::CatapultState dummyState;
			auto observerContext = observers::ObserverContext(cache, dummyState, effectiveHeight, observers::NotifyMode::Commit);
			for (auto& info : unconfirmedTransactionInfos) {
				if (!filter(info))
					continue;

				const auto& entity = *info.pEntity;
				auto entityHash = info.EntityHash;
				if (!applyState.Modifier.add(std::move(info)))
					continue;

				// notice that subscriber is created within loop because aggregate result needs to be reset each iteration
				ProcessingNotificationSubscriber sub(*m_config.pValidator, validatorContext, *m_config.pObserver, observerContext);
				sub.enableUndo();
				auto entityInfo = model::WeakEntityInfo(entity, entityHash);
				m_config.pNotificationPublisher->publish(entityInfo, sub);
				if (!IsValidationResultSuccess(sub.result())) {
					CATAPULT_LOG_LEVEL(validators::MapToLogLevel(sub.result()))
							<< "dropping transaction " << utils::HexFormat(entityHash) << ": " << sub.result();
					sub.undo();
					applyState.Modifier.remove(entityHash);
					continue;
				}
			}
		}

		void addAll(
				cache::UtCacheModifierProxy& modifier,
				std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
			for (auto& info : unconfirmedTransactionInfos)
				modifier.add(std::move(info));
		}

	private:
		cache::UtCache& m_transactionsCache;
		const cache::CatapultCache& m_confirmedCatapultCache;
		ExecutionConfiguration m_config;
		TimeProvider m_timeProvider;
		Height m_cacheHeight;
		std::unique_ptr<DetachedDeltaWrapper> m_pLockableUnconfirmedCatapultCache;
	};

	UnconfirmedTransactionsUpdater::UnconfirmedTransactionsUpdater(
			cache::UtCache& transactionsCache,
			const cache::CatapultCache& confirmedCatapultCache,
			const ExecutionConfiguration& config,
			const std::function<Timestamp ()>& timeProvider)
			: m_pImpl(std::make_unique<Impl>(transactionsCache, confirmedCatapultCache, config, timeProvider))
	{}

	UnconfirmedTransactionsUpdater::~UnconfirmedTransactionsUpdater() = default;

	void UnconfirmedTransactionsUpdater::update(std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
		m_pImpl->update(std::move(unconfirmedTransactionInfos));
	}

	void UnconfirmedTransactionsUpdater::update(
			const utils::HashPointerSet& confirmedTransactionHashes,
			std::vector<model::TransactionInfo>&& unconfirmedTransactionInfos) {
		m_pImpl->update(confirmedTransactionHashes, std::move(unconfirmedTransactionInfos));
	}
}}
