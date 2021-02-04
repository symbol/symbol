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

#include "MultiBlockLoader.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/extensions/LocalNodeStateRef.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/Elements.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace local {

	// region CreateBlockDependentNotificationObserverFactory

	namespace {
		class SkipTransientStatePredicate {
		public:
			SkipTransientStatePredicate(const model::Block& lastBlock, const model::BlockChainConfiguration& config)
					: m_inflectionTime(DifferenceOrZero(
							lastBlock.Timestamp.unwrap(),
							model::CalculateTransactionCacheDuration(config).millis()))
					, m_inflectionHeight(DifferenceOrZero(lastBlock.Height.unwrap(), config.MaxDifficultyBlocks) + 1)
			{}

		public:
			bool operator()(const model::Block& block) const {
				// only skip blocks before inflection points
				return m_inflectionTime > block.Timestamp && m_inflectionHeight > block.Height;
			}

		private:
			static constexpr uint64_t DifferenceOrZero(uint64_t lhs, uint64_t rhs) {
				return lhs > rhs ? lhs - rhs : 0;
			}

		private:
			Timestamp m_inflectionTime;
			Height m_inflectionHeight;
		};
	}

	BlockDependentNotificationObserverFactory CreateBlockDependentNotificationObserverFactory(
			const model::Block& lastBlock,
			const model::BlockChainConfiguration& config,
			const NotificationObserverFactory& transientObserverFactory,
			const NotificationObserverFactory& permanentObserverFactory) {
		auto predicate = SkipTransientStatePredicate(lastBlock, config);
		return [predicate, transientObserverFactory, permanentObserverFactory](const auto& block) {
			return predicate(block) ? permanentObserverFactory() : transientObserverFactory();
		};
	}

	// endregion

	// region LoadBlockChain

	namespace {
		class AnalyzeProgressLogger {
		private:
			static constexpr auto Log_Interval_Millis = 2'000;

		public:
			explicit AnalyzeProgressLogger(const utils::StackTimer& stopwatch) : m_stopwatch(stopwatch), m_numLogs(0)
			{}

		public:
			void operator()(Height height, Height chainHeight) {
				auto currentMillis = m_stopwatch.millis();
				if (currentMillis < (m_numLogs + 1) * Log_Interval_Millis)
					return;

				CATAPULT_LOG(info) << "loaded " << height << " / " << chainHeight << " blocks in " << currentMillis << "ms";
				++m_numLogs;
			}

		private:
			const utils::StackTimer& m_stopwatch;
			size_t m_numLogs;
		};
	}

	class BlockChainLoader {
	private:
		using NotifyProgressFunc = consumer<Height, Height>;

	public:
		BlockChainLoader(
				const BlockDependentNotificationObserverFactory& observerFactory,
				const plugins::PluginManager& pluginManager,
				const extensions::LocalNodeStateRef& stateRef,
				Height startHeight,
				const consumer<LoadedBlockStatus&&>& statusConsumer)
				: m_observerFactory(observerFactory)
				, m_pluginManager(pluginManager)
				, m_stateRef(stateRef)
				, m_startHeight(startHeight)
				, m_statusConsumer(statusConsumer)
		{}

	public:
		model::ChainScore loadAll(const NotifyProgressFunc& notifyProgress) const {
			const auto& storage = m_stateRef.Storage.view();

			auto height = m_startHeight;
			auto pParentBlockElement = storage.loadBlockElement(height - Height(1));

			model::ChainScore previousScore;
			model::ChainScore score;
			Hash256 stateHash;
			auto chainHeight = storage.chainHeight();
			while (chainHeight >= height) {
				auto pBlockElement = storage.loadBlockElement(height);
				score += model::ChainScore(chain::CalculateScore(pParentBlockElement->Block, pBlockElement->Block));

				const auto& blockElement = *pBlockElement;
				const auto& statusConsumer = m_statusConsumer;
				stateHash = execute(blockElement, [&previousScore, &score, &blockElement, &statusConsumer](auto&& cacheChanges) {
					if (!statusConsumer)
						return;

					auto scoreDelta = score - previousScore;
					auto stateChangeInfo = subscribers::StateChangeInfo{ std::move(cacheChanges), scoreDelta, blockElement.Block.Height };
					statusConsumer(LoadedBlockStatus{ blockElement, score, stateChangeInfo });
				});
				notifyProgress(height, chainHeight);

				pParentBlockElement = std::move(pBlockElement);
				previousScore = score;
				height = height + Height(1);
			}

			if (chainHeight >= m_startHeight) {
				CATAPULT_LOG(info)
						<< "cache state hash at height " << chainHeight << ": " << stateHash
						<< " (loaded from height " << m_startHeight << ")";
			}

			return score;
		}

	private:
		Hash256 execute(const model::BlockElement& blockElement, const consumer<cache::CacheChanges&&>& cacheChangesConsumer) const {
			auto cacheDelta = m_stateRef.Cache.createDelta();
			auto observerState = observers::ObserverState(cacheDelta);

			auto readOnlyCache = cacheDelta.toReadOnly();
			auto resolverContext = m_pluginManager.createResolverContext(readOnlyCache);

			const auto& block = blockElement.Block;
			observers::NotificationObserverAdapter observer(m_observerFactory(block), m_pluginManager.createNotificationPublisher());
			chain::ExecuteBlock(blockElement, { observer, resolverContext, observerState });

			// populate patricia tree delta
			auto stateHash = cacheDelta.calculateStateHash(block.Height).StateHash;

			cacheChangesConsumer(cache::CacheChanges(cacheDelta));

			m_stateRef.Cache.commit(block.Height);
			return stateHash;
		}

	private:
		BlockDependentNotificationObserverFactory m_observerFactory;
		const plugins::PluginManager& m_pluginManager;
		const extensions::LocalNodeStateRef& m_stateRef;
		Height m_startHeight;
		consumer<LoadedBlockStatus&&> m_statusConsumer;
	};

	model::ChainScore LoadBlockChain(
			const BlockDependentNotificationObserverFactory& observerFactory,
			const plugins::PluginManager& pluginManager,
			const extensions::LocalNodeStateRef& stateRef,
			Height startHeight,
			const consumer<LoadedBlockStatus&&>& statusConsumer) {
		BlockChainLoader loader(observerFactory, pluginManager, stateRef, startHeight, statusConsumer);

		utils::StackLogger logger("load block chain", utils::LogLevel::important);
		utils::StackTimer stopwatch;
		return loader.loadAll(AnalyzeProgressLogger(stopwatch));
	}

	// endregion
}}
