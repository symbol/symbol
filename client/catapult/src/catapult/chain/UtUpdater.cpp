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

#include "UtUpdater.h"
#include "ChainResults.h"
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
				const FailedTransactionSink& failedTransactionSink,
				const Throttle& throttle)
				: m_transactionsCache(transactionsCache)
				, m_detachedCatapultCache(confirmedCatapultCache)
				, m_config(config)
				, m_timeSupplier(timeSupplier)
				, m_failedTransactionSink(failedTransactionSink)
				, m_throttle(throttle)
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
			apply(applyState, utInfos, TransactionSource::New);
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
			apply(applyState, utInfos, TransactionSource::Reverted);

			// 4. add back original txes that have not been confirmed
			apply(applyState, originalTransactionInfos, TransactionSource::Existing, [&confirmedTransactionHashes](const auto& info) {
				return confirmedTransactionHashes.cend() == confirmedTransactionHashes.find(&info.EntityHash);
			});
		}

	private:
		void apply(const ApplyState& applyState, const std::vector<model::TransactionInfo>& utInfos, TransactionSource transactionSource) {
			apply(applyState, utInfos, transactionSource, [](const auto&) { return true; });
		}

		void apply(
				const ApplyState& applyState,
				const std::vector<model::TransactionInfo>& utInfos,
				TransactionSource transactionSource,
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
			for (const auto& utInfo : utInfos) {
				const auto& entity = *utInfo.pEntity;
				const auto& entityHash = utInfo.EntityHash;

				if (!filter(utInfo))
					continue;

				if (throttle(utInfo, transactionSource, applyState, readOnlyCache)) {
					CATAPULT_LOG(warning) << "dropping transaction " << utils::HexFormat(entityHash) << " due to throttle";
					m_failedTransactionSink(entity, entityHash, Failure_Chain_Unconfirmed_Cache_Too_Full);
					continue;
				}

				if (!applyState.Modifier.add(utInfo))
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

		bool throttle(
				const model::TransactionInfo& utInfo,
				TransactionSource transactionSource,
				const ApplyState& applyState,
				cache::ReadOnlyCatapultCache& cache) const {
			return m_throttle(utInfo, { transactionSource, m_detachedCatapultCache.height(), cache, applyState.Modifier });
		}

		void addAll(cache::UtCacheModifierProxy& modifier, const std::vector<model::TransactionInfo>& utInfos) {
			for (const auto& utInfo : utInfos)
				modifier.add(utInfo);
		}

	private:
		cache::UtCache& m_transactionsCache;
		cache::RelockableDetachedCatapultCache m_detachedCatapultCache;
		ExecutionConfiguration m_config;
		TimeSupplier m_timeSupplier;
		FailedTransactionSink m_failedTransactionSink;
		UtUpdater::Throttle m_throttle;
	};

	UtUpdater::UtUpdater(
			cache::UtCache& transactionsCache,
			const cache::CatapultCache& confirmedCatapultCache,
			const ExecutionConfiguration& config,
			const TimeSupplier& timeSupplier,
			const FailedTransactionSink& failedTransactionSink,
			const Throttle& throttle)
			: m_pImpl(std::make_unique<Impl>(
					transactionsCache,
					confirmedCatapultCache,
					config,
					timeSupplier,
					failedTransactionSink,
					throttle))
	{}

	UtUpdater::~UtUpdater() = default;

	void UtUpdater::update(const std::vector<model::TransactionInfo>& utInfos) {
		m_pImpl->update(utInfos);
	}

	void UtUpdater::update(const utils::HashPointerSet& confirmedTransactionHashes, const std::vector<model::TransactionInfo>& utInfos) {
		m_pImpl->update(confirmedTransactionHashes, utInfos);
	}
}}
