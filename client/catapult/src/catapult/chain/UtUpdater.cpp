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
#include "ProcessContextsBuilder.h"
#include "ProcessingNotificationSubscriber.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/RelockableDetachedCatapultCache.h"
#include "catapult/cache_tx/UtCache.h"
#include "catapult/model/FeeUtils.h"
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

		class TransactionInfoFormatter {
		public:
			explicit TransactionInfoFormatter(const model::TransactionInfo& transactionInfo) : m_transactionInfo(transactionInfo)
			{}

		public:
			friend std::ostream& operator<<(std::ostream& out, const TransactionInfoFormatter& formatter) {
				const auto& transaction = *formatter.m_transactionInfo.pEntity;
				out
						<< "{ EntityHash " << formatter.m_transactionInfo.EntityHash
						<< ", Type " << transaction.Type
						<< ", Version " << static_cast<uint16_t>(transaction.Version)
						<< ", SignerPublicKey " << transaction.SignerPublicKey
						<< " }";
				return out;
			}

		private:
			const model::TransactionInfo& m_transactionInfo;
		};
	}

	class UtUpdater::Impl final {
	public:
		Impl(
				cache::UtCache& transactionsCache,
				const cache::CatapultCache& confirmedCatapultCache,
				BlockFeeMultiplier minFeeMultiplier,
				const ExecutionConfiguration& executionConfig,
				const TimeSupplier& timeSupplier,
				const FailedTransactionSink& failedTransactionSink,
				const Throttle& throttle)
				: m_transactionsCache(transactionsCache)
				, m_detachedCatapultCache(confirmedCatapultCache)
				, m_minFeeMultiplier(minFeeMultiplier)
				, m_executionConfig(executionConfig)
				, m_timeSupplier(timeSupplier)
				, m_failedTransactionSink(failedTransactionSink)
				, m_throttle(throttle)
		{}

	public:
		void update(const std::vector<model::TransactionInfo>& utInfos) {
			// 1. lock the UT cache and lock the unconfirmed copy
			auto modifier = m_transactionsCache.modifier();
			auto pUnconfirmedCatapultCache = m_detachedCatapultCache.getAndTryLock();
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

			// 1. lock and clear the UT cache - UT cache must be locked before catapult cache to prevent race condition whereby
			//    other update overload applies transactions to rebased cache before UT lock is held
			auto modifier = m_transactionsCache.modifier();
			auto originalTransactionInfos = modifier.removeAll();

			// 2. lock the catapult cache and rebase the unconfirmed catapult cache
			auto pUnconfirmedCatapultCache = m_detachedCatapultCache.rebaseAndLock();

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
			// note that the validator and observer context height is one larger than the chain height
			// since the validation and observation has to be for the *next* block
			auto effectiveHeight = m_detachedCatapultCache.height() + Height(1);
			ProcessContextsBuilder contextBuilder(effectiveHeight, m_timeSupplier(), m_executionConfig);
			contextBuilder.setCache(applyState.UnconfirmedCatapultCache);
			auto validatorContext = contextBuilder.buildValidatorContext();
			auto observerContext = contextBuilder.buildObserverContext();

			for (const auto& utInfo : utInfos) {
				const auto& entity = *utInfo.pEntity;
				const auto& entityHash = utInfo.EntityHash;

				if (!filter(utInfo))
					continue;

				auto minTransactionFee = model::CalculateTransactionFee(m_minFeeMultiplier, entity);
				if (entity.MaxFee < minTransactionFee) {
					// don't log reverted transactions that could have been included by harvester with lower min fee multiplier
					if (TransactionSource::New == transactionSource) {
						CATAPULT_LOG(debug)
								<< "dropping transaction " << TransactionInfoFormatter(utInfo) << " with max fee " << entity.MaxFee
								<< " because min fee is " << minTransactionFee;
					}

					continue;
				}

				if (throttle(utInfo, transactionSource, applyState, validatorContext.Cache)) {
					CATAPULT_LOG(warning) << "dropping transaction " << TransactionInfoFormatter(utInfo) << " due to throttle";
					m_failedTransactionSink(entity, entityHash, Failure_Chain_Unconfirmed_Cache_Too_Full);
					continue;
				}

				if (!applyState.Modifier.add(utInfo))
					continue;

				// notice that subscriber is created within loop because aggregate result needs to be reset each iteration
				const auto& validator = *m_executionConfig.pValidator;
				const auto& observer = *m_executionConfig.pObserver;
				ProcessingNotificationSubscriber sub(validator, validatorContext, observer, observerContext);
				sub.enableUndo();
				auto entityInfo = model::WeakEntityInfo(entity, entityHash);
				m_executionConfig.pNotificationPublisher->publish(entityInfo, sub);
				if (!IsValidationResultSuccess(sub.result())) {
					CATAPULT_LOG_LEVEL(validators::MapToLogLevel(sub.result()))
							<< "dropping transaction " << TransactionInfoFormatter(utInfo) << ": " << sub.result();

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
				const cache::ReadOnlyCatapultCache& cache) const {
			return m_throttle(utInfo, { transactionSource, m_detachedCatapultCache.height(), cache, applyState.Modifier });
		}

		void addAll(cache::UtCacheModifierProxy& modifier, const std::vector<model::TransactionInfo>& utInfos) {
			for (const auto& utInfo : utInfos)
				modifier.add(utInfo);
		}

	private:
		cache::UtCache& m_transactionsCache;
		cache::RelockableDetachedCatapultCache m_detachedCatapultCache;
		BlockFeeMultiplier m_minFeeMultiplier;
		ExecutionConfiguration m_executionConfig;
		TimeSupplier m_timeSupplier;
		FailedTransactionSink m_failedTransactionSink;
		UtUpdater::Throttle m_throttle;
	};

	UtUpdater::UtUpdater(
			cache::UtCache& transactionsCache,
			const cache::CatapultCache& confirmedCatapultCache,
			BlockFeeMultiplier minFeeMultiplier,
			const ExecutionConfiguration& executionConfig,
			const TimeSupplier& timeSupplier,
			const FailedTransactionSink& failedTransactionSink,
			const Throttle& throttle)
			: m_pImpl(std::make_unique<Impl>(
					transactionsCache,
					confirmedCatapultCache,
					minFeeMultiplier,
					executionConfig,
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
