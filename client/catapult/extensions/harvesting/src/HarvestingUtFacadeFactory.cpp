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

#include "HarvestingUtFacadeFactory.h"
#include "HarvestingCacheUtils.h"
#include "HarvestingObservers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/ProcessContextsBuilder.h"
#include "catapult/chain/ProcessingNotificationSubscriber.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/BlockchainConfiguration.h"
#include "catapult/model/FeeUtils.h"
#include "catapult/model/VotingSet.h"
#include "catapult/observers/DemuxObserverBuilder.h"

namespace catapult { namespace harvesting {

	namespace {
		// region NotificationObserverProxy

		class NotificationObserverProxy : public observers::NotificationObserver {
		public:
			explicit NotificationObserverProxy(const std::shared_ptr<const observers::NotificationObserver>& pObserver)
					: m_pObserver(pObserver) {
			}

		public:
			const std::string& name() const override {
				return m_pObserver->name();
			}

			void notify(const model::Notification& notification, observers::ObserverContext& context) const override {
				return m_pObserver->notify(notification, context);
			}

		private:
			std::shared_ptr<const observers::NotificationObserver> m_pObserver;
		};

		// endregion

		// region ImportanceFilteringNotificationSubscriber

		class ImportanceFilteringNotificationSubscriber : public model::NotificationSubscriber {
		public:
			explicit ImportanceFilteringNotificationSubscriber(model::NotificationSubscriber& subscriber)
					: m_subscriber(subscriber) {
			}

		public:
			void notify(const model::Notification& notification) override {
				// harvester fills in block importance *after* execution, so skip these notifications (only used for validation)
				if (model::Core_Block_Importance_Notification == notification.Type)
					return;

				m_subscriber.notify(notification);
			}

		private:
			model::NotificationSubscriber& m_subscriber;
		};

		// endregion
	}

	// region HarvestingUtFacade::Impl

	class HarvestingUtFacade::Impl {
	private:
		class CacheFacade {
		public:
			explicit CacheFacade(const cache::CatapultCache& cache)
					: m_cacheDetachableDelta(cache.createDetachableDelta())
					, m_cacheDetachedDelta(m_cacheDetachableDelta.detach())
					, m_pCacheDelta(m_cacheDetachedDelta.tryLock()) {
			}

		public:
			Height height() {
				return m_cacheDetachableDelta.height();
			}

			cache::CatapultCacheDelta& delta() {
				return *m_pCacheDelta;
			}

		private:
			cache::CatapultCacheDetachableDelta m_cacheDetachableDelta;
			cache::CatapultCacheDetachedDelta m_cacheDetachedDelta;
			std::unique_ptr<cache::CatapultCacheDelta> m_pCacheDelta;
		};

	public:
		Impl(Timestamp blockTime,
			 const cache::CatapultCache& cache,
			 const model::BlockchainConfiguration& blockchainConfig,
			 const chain::ExecutionConfiguration& executionConfig,
			 const ImportanceBlockHashSupplier& importanceBlockHashSupplier)
				: m_blockTime(blockTime)
				, m_blockchainConfig(blockchainConfig)
				, m_executionConfig(executionConfig)
				, m_importanceBlockHashSupplier(importanceBlockHashSupplier)
				, m_pCacheFacade(std::make_unique<CacheFacade>(cache))
				, m_cacheHeight(m_pCacheFacade->height()) {
			// add additional observers to monitor accounts
			observers::DemuxObserverBuilder observerBuilder;
			observerBuilder.add(CreateHarvestingAccountAddressObserver(m_affectedAccounts));
			observerBuilder.add(CreateHarvestingAccountPublicKeyObserver(m_affectedAccounts));
			observerBuilder.add<model::Notification>(std::make_unique<NotificationObserverProxy>(m_executionConfig.pObserver));
			m_executionConfig.pObserver = observerBuilder.build();
		}

	public:
		Height height() const {
			return m_cacheHeight + Height(1);
		}

	public:
		bool apply(const model::TransactionInfo& transactionInfo) {
			auto originalSource = m_blockStatementBuilder.source();

			model::BlockHeader blockHeader;
			setBlockHeaderFieldsForApply(blockHeader);

			if (apply(model::WeakEntityInfo(*transactionInfo.pEntity, transactionInfo.EntityHash, blockHeader)))
				return true;

			auto finalSource = m_blockStatementBuilder.source();
			if (originalSource.PrimaryId != finalSource.PrimaryId)
				m_blockStatementBuilder.popSource();

			return false;
		}

		void unapply(const model::TransactionInfo& transactionInfo) {
			model::BlockHeader blockHeader;
			setBlockHeaderFieldsForApply(blockHeader);

			unapply(model::WeakEntityInfo(*transactionInfo.pEntity, transactionInfo.EntityHash, blockHeader));
			m_blockStatementBuilder.popSource();
		}

		std::unique_ptr<model::Block> commit(const model::BlockHeader& blockHeader, const model::Transactions& transactions) {
			if (!m_pCacheFacade)
				CATAPULT_THROW_INVALID_ARGUMENT("facade has already committed a block");

			// 1. stitch block
			auto pBlock = model::StitchBlock(blockHeader, transactions);
			auto importanceHeight = model::ConvertToImportanceHeight(pBlock->Height, m_blockchainConfig.ImportanceGrouping);

			// 2. add back fee surpluses to accounts (skip cache lookup if no surplus)
			auto& accountStateCacheDelta = m_pCacheFacade->delta().sub<cache::AccountStateCache>();
			for (const auto& transaction : pBlock->Transactions()) {
				auto surplus = transaction.MaxFee - model::CalculateTransactionFee(blockHeader.FeeMultiplier, transaction);
				if (Amount(0) != surplus) {
					auto accountStateIter = accountStateCacheDelta.find(transaction.SignerPublicKey);
					state::ApplyFeeSurplus(accountStateIter.get(), { m_blockchainConfig.CurrencyMosaicId, surplus }, importanceHeight);
				}
			}

			// 3. execute block (using zero hash)
			if (!apply(model::WeakEntityInfo(*pBlock, Hash256())))
				return nullptr;

			// 4. update account states
			updateAccountStates(accountStateCacheDelta);

			// 5. update importance block fields
			if (model::IsImportanceBlock(pBlock->Type)) {
				accountStateCacheDelta.updateHighValueAccounts(pBlock->Height);

				auto epoch = pBlock->Height < m_blockchainConfig.ForkHeights.TotalVotingBalanceCalculationFix
									 ? FinalizationEpoch(0)
									 : model::CalculateFinalizationEpochForHeight(pBlock->Height, m_blockchainConfig.VotingSetGrouping);

				auto statistics = cache::ReadOnlyAccountStateCache(accountStateCacheDelta).highValueAccountStatistics(epoch);
				auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(*pBlock);
				blockFooter.VotingEligibleAccountsCount = statistics.VotingEligibleAccountsCount;
				blockFooter.HarvestingEligibleAccountsCount = statistics.HarvestingEligibleAccountsCount;
				blockFooter.TotalVotingBalance = statistics.TotalVotingBalance;
			}

			// 6. update block fields
			pBlock->StateHash =
					m_blockchainConfig.EnableVerifiableState ? m_pCacheFacade->delta().calculateStateHash(height()).StateHash : Hash256();

			pBlock->ReceiptsHash =
					m_blockchainConfig.EnableVerifiableReceipts ? model::CalculateMerkleHash(*m_blockStatementBuilder.build()) : Hash256();

			// 7. update PreviousImportanceBlockHash after releasing cache lock to avoid potential deadlock
			m_pCacheFacade.reset();
			if (model::IsImportanceBlock(pBlock->Type)) {
				// assume block has height that is multiple of importance grouping
				model::HeightGroupingFacade<Height> groupingFacade(pBlock->Height, m_blockchainConfig.ImportanceGrouping);

				auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(*pBlock);
				blockFooter.PreviousImportanceBlockHash = m_importanceBlockHashSupplier(groupingFacade.previous(1));
			}

			return pBlock;
		}

	private:
		void setBlockHeaderFieldsForApply(model::BlockHeader& blockHeader) const {
			blockHeader.Height = height();

			// indicate transaction MaxFee should be used (this works because the associated block header is not validated)
			blockHeader.VerifiableEntityHeader_Reserved1 = 1;
		}

		using Processor = predicate<
				const validators::stateful::NotificationValidator&,
				const validators::ValidatorContext&,
				const observers::NotificationObserver&,
				observers::ObserverContext&>;

		bool process(const Processor& processor) {
			// prepare state and contexts
			chain::ProcessContextsBuilder contextBuilder(height(), m_blockTime, m_executionConfig);
			contextBuilder.setCache(m_pCacheFacade->delta());
			if (m_blockchainConfig.EnableVerifiableReceipts)
				contextBuilder.setBlockStatementBuilder(m_blockStatementBuilder);

			auto validatorContext = contextBuilder.buildValidatorContext();
			auto observerContext = contextBuilder.buildObserverContext();

			const auto& validator = *m_executionConfig.pValidator;
			const auto& observer = *m_executionConfig.pObserver;
			return processor(validator, validatorContext, observer, observerContext);
		}

		bool apply(const model::WeakEntityInfo& weakEntityInfo) {
			const auto& publisher = *m_executionConfig.pNotificationPublisher;
			return process([&weakEntityInfo,
							&publisher](const auto& validator, const auto& validatorContext, const auto& observer, auto& observerContext) {
				chain::ProcessingNotificationSubscriber sub(validator, validatorContext, observer, observerContext);
				sub.enableUndo();

				// execute entity
				if (model::IsImportanceBlock(weakEntityInfo.type())) {
					ImportanceFilteringNotificationSubscriber filteringSub(sub);
					publisher.publish(weakEntityInfo, filteringSub);
				} else {
					publisher.publish(weakEntityInfo, sub);
				}

				if (validators::IsValidationResultSuccess(sub.result()))
					return true;

				sub.undo();
				return false;
			});
		}

		void unapply(const model::WeakEntityInfo& weakEntityInfo) {
			const auto& publisher = *m_executionConfig.pNotificationPublisher;
			process([&weakEntityInfo, &publisher](const auto&, const auto&, const auto& observer, auto& observerContext) {
				chain::ProcessingUndoNotificationSubscriber sub(observer, observerContext);

				// execute entity
				publisher.publish(weakEntityInfo, sub);
				sub.undo();
				return true;
			});
		}

		void updateAccountStates(cache::AccountStateCacheDelta& accountStateCacheDelta) {
			PreserveAllAccounts(accountStateCacheDelta, m_affectedAccounts, height());
			accountStateCacheDelta.commitRemovals(cache::AccountStateCacheDelta::CommitRemovalsMode::Unlinked);
		}

	private:
		Timestamp m_blockTime;
		model::BlockchainConfiguration m_blockchainConfig;
		chain::ExecutionConfiguration m_executionConfig;
		ImportanceBlockHashSupplier m_importanceBlockHashSupplier;

		std::unique_ptr<CacheFacade> m_pCacheFacade;
		Height m_cacheHeight;

		model::BlockStatementBuilder m_blockStatementBuilder;
		HarvestingAffectedAccounts m_affectedAccounts;
	};

	// endregion

	// region HarvestingUtFacade

	HarvestingUtFacade::HarvestingUtFacade(
			Timestamp blockTime,
			const cache::CatapultCache& cache,
			const model::BlockchainConfiguration& blockchainConfig,
			const chain::ExecutionConfiguration& executionConfig,
			const ImportanceBlockHashSupplier& importanceBlockHashSupplier)
			: m_pImpl(std::make_unique<Impl>(blockTime, cache, blockchainConfig, executionConfig, importanceBlockHashSupplier)) {
	}

	HarvestingUtFacade::~HarvestingUtFacade() = default;

	Height HarvestingUtFacade::height() const {
		return m_pImpl->height();
	}

	size_t HarvestingUtFacade::size() const {
		return m_transactionInfos.size();
	}

	const std::vector<model::TransactionInfo>& HarvestingUtFacade::transactionInfos() const {
		return m_transactionInfos;
	}

	bool HarvestingUtFacade::apply(const model::TransactionInfo& transactionInfo) {
		if (!m_pImpl->apply(transactionInfo))
			return false;

		m_transactionInfos.push_back(transactionInfo.copy());
		return true;
	}

	void HarvestingUtFacade::unapply() {
		if (m_transactionInfos.empty())
			CATAPULT_THROW_OUT_OF_RANGE("cannot call unapply when no transactions have been applied");

		m_pImpl->unapply(m_transactionInfos.back());
		m_transactionInfos.pop_back();
	}

	std::unique_ptr<model::Block> HarvestingUtFacade::commit(const model::BlockHeader& blockHeader) {
		if (height() != blockHeader.Height)
			CATAPULT_THROW_RUNTIME_ERROR("commit block header is inconsistent with facade state");

		model::Transactions transactions;
		for (const auto& transactionInfo : m_transactionInfos)
			transactions.push_back(transactionInfo.pEntity);

		auto pBlock = m_pImpl->commit(blockHeader, transactions);
		m_transactionInfos.clear();
		return pBlock;
	}

	// endregion

	// region HarvestingUtFacadeFactory

	HarvestingUtFacadeFactory::HarvestingUtFacadeFactory(
			const cache::CatapultCache& cache,
			const model::BlockchainConfiguration& blockchainConfig,
			const chain::ExecutionConfiguration& executionConfig,
			const ImportanceBlockHashSupplier& importanceBlockHashSupplier)
			: m_cache(cache)
			, m_blockchainConfig(blockchainConfig)
			, m_executionConfig(executionConfig)
			, m_importanceBlockHashSupplier(importanceBlockHashSupplier) {
	}

	std::unique_ptr<HarvestingUtFacade> HarvestingUtFacadeFactory::create(Timestamp blockTime) const {
		return std::make_unique<HarvestingUtFacade>(
				blockTime,
				m_cache,
				m_blockchainConfig,
				m_executionConfig,
				m_importanceBlockHashSupplier);
	}

	// endregion
}}
