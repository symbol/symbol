#include "UtUpdater.h"
#include "ProcessingNotificationSubscriber.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/RelockableDetachedCatapultCache.h"
#include "catapult/cache/UtCache.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace chain {

	namespace {
		struct ApplyState {
			constexpr ApplyState(cache::UtCacheModifierProxy& modifier, cache::CatapultCacheDelta& unconfirmedCatapultCache)
					: Modifier(modifier)
					, UnconfirmedCatapultCache(unconfirmedCatapultCache)
			{}

			cache::UtCacheModifierProxy& Modifier;
			cache::CatapultCacheDelta& UnconfirmedCatapultCache;
		};
	}

	class UtUpdater::Impl final {
	public:
		Impl(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				const ExecutionConfiguration& config,
				const TimeSupplier& timeSupplier,
				const FailedTransactionSink& failedTransactionSink)
				: m_transactionsCache(transactionsCache)
				, m_detachedCatapultCache(confirmedCatapultCache)
				, m_config(config)
				, m_timeSupplier(timeSupplier)
				, m_failedTransactionSink(failedTransactionSink)
		{}

	public:
		void update(const std::vector<model::TransactionInfo>& utInfos) {
			// 1. lock the UT cache and lock the unconfirmed copy
			auto modifier = m_transactionsCache.modifier();
			auto pUnconfirmedCatapultCache = m_detachedCatapultCache.getAndLock();
			if (!pUnconfirmedCatapultCache) {
				// if there is no unconfirmed cache state, it means that a block update is forthcoming
				// just add all to the cache and they will be validated later
				addAll(modifier, utInfos);
				return;
			}

			auto applyState = ApplyState(modifier, *pUnconfirmedCatapultCache);
			apply(applyState, utInfos);
		}

		void update(const utils::HashPointerSet& confirmedTransactionHashes, const std::vector<model::TransactionInfo>& utInfos) {
			if (!confirmedTransactionHashes.empty() || !utInfos.empty()) {
				CATAPULT_LOG(debug)
						<< "confirmed " << confirmedTransactionHashes.size() << " transactions, "
						<< "reverted " << utInfos.size() << " transactions";
			}

			// 1. lock the catapult cache and rebase the unconfirmed catapult cache
			auto pUnconfirmedCatapultCache = m_detachedCatapultCache.rebaseAndLock();

			// 2. lock and clear the UT cache
			auto modifier = m_transactionsCache.modifier();
			auto originalTransactionInfos = modifier.removeAll();

			// 3. add back reverted txes
			auto applyState = ApplyState(modifier, *pUnconfirmedCatapultCache);
			apply(applyState, utInfos);

			// 4. add back original txes that have not been confirmed
			apply(applyState, originalTransactionInfos, [&confirmedTransactionHashes](const auto& info) {
				return confirmedTransactionHashes.cend() == confirmedTransactionHashes.find(&info.EntityHash);
			});
		}

	private:
		void apply(const ApplyState& applyState, const std::vector<model::TransactionInfo>& utInfos) {
			apply(applyState, utInfos, [](const auto&) { return true; });
		}

		void apply(
				const ApplyState& applyState,
				const std::vector<model::TransactionInfo>& utInfos,
				const predicate<const model::TransactionInfo&>& filter) {
			auto currentTime = m_timeSupplier();

			auto readOnlyCache = applyState.UnconfirmedCatapultCache.toReadOnly();

			// note that the validator and observer context height is one larger than the chain height
			// since the validation and observation has to be for the *next* block
			auto effectiveHeight = m_detachedCatapultCache.height() + Height(1);
			auto validatorContext = validators::ValidatorContext(effectiveHeight, currentTime, m_config.Network, readOnlyCache);

			// note that the "real" state is currently only required by block observers, so a dummy state can be used
			auto& cache = applyState.UnconfirmedCatapultCache;
			state::CatapultState dummyState;
			auto observerContext = observers::ObserverContext(cache, dummyState, effectiveHeight, observers::NotifyMode::Commit);
			for (const auto& info : utInfos) {
				if (!filter(info))
					continue;

				const auto& entity = *info.pEntity;
				const auto& entityHash = info.EntityHash;
				if (!applyState.Modifier.add(info))
					continue;

				// notice that subscriber is created within loop because aggregate result needs to be reset each iteration
				ProcessingNotificationSubscriber sub(*m_config.pValidator, validatorContext, *m_config.pObserver, observerContext);
				sub.enableUndo();
				auto entityInfo = model::WeakEntityInfo(entity, entityHash);
				m_config.pNotificationPublisher->publish(entityInfo, sub);
				if (!IsValidationResultSuccess(sub.result())) {
					CATAPULT_LOG_LEVEL(validators::MapToLogLevel(sub.result()))
							<< "dropping transaction " << utils::HexFormat(entityHash) << ": " << sub.result();

					// only forward failure (not neutral) results
					if (IsValidationResultFailure(sub.result()))
						m_failedTransactionSink(entity, entityHash, sub.result());

					sub.undo();
					applyState.Modifier.remove(entityHash);
					continue;
				}
			}
		}

		void addAll(cache::UtCacheModifierProxy& modifier, const std::vector<model::TransactionInfo>& utInfos) {
			for (const auto& info : utInfos)
				modifier.add(info);
		}

	private:
		cache::UtCache& m_transactionsCache;
		cache::RelockableDetachedCatapultCache m_detachedCatapultCache;
		ExecutionConfiguration m_config;
		TimeSupplier m_timeSupplier;
		FailedTransactionSink m_failedTransactionSink;
	};

	UtUpdater::UtUpdater(
			cache::UtCache& transactionsCache,
			const cache::CatapultCache& confirmedCatapultCache,
			const ExecutionConfiguration& config,
			const TimeSupplier& timeSupplier,
			const FailedTransactionSink& failedTransactionSink)
			: m_pImpl(std::make_unique<Impl>(transactionsCache, confirmedCatapultCache, config, timeSupplier, failedTransactionSink))
	{}

	UtUpdater::~UtUpdater() = default;

	void UtUpdater::update(const std::vector<model::TransactionInfo>& utInfos) {
		m_pImpl->update(utInfos);
	}

	void UtUpdater::update(const utils::HashPointerSet& confirmedTransactionHashes, const std::vector<model::TransactionInfo>& utInfos) {
		m_pImpl->update(confirmedTransactionHashes, utInfos);
	}
}}
