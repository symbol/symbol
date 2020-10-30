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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/ChainScore.h"
#include "tests/catapult/consumers/test/ConsumerInputFactory.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/cache/UnsupportedSubCachePlugin.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;
using catapult::disruptor::InputSource;
using catapult::utils::HashSet;
using catapult::validators::ValidationResult;

namespace catapult { namespace consumers {

#define TEST_CLASS BlockChainSyncConsumerTests

	namespace {
		constexpr auto Base_Difficulty = Difficulty().unwrap();

		constexpr model::ImportanceHeight Initial_Last_Recalculation_Height(1234);
		constexpr model::ImportanceHeight Modified_Last_Recalculation_Height(7777);
		const Key Sentinel_Processor_Public_Key = test::GenerateRandomByteArray<Key>();

		constexpr model::ImportanceHeight AddImportanceHeight(model::ImportanceHeight lhs, model::ImportanceHeight::ValueType rhs) {
			return model::ImportanceHeight(lhs.unwrap() + rhs);
		}

		// region RaisableErrorSource

		class RaisableErrorSource {
		public:
			RaisableErrorSource() : m_shouldRaiseError(false)
			{}

		public:
			void setError() {
				m_shouldRaiseError = true;
			}

		protected:
			void raise(const std::string& source) const {
				if (m_shouldRaiseError)
					CATAPULT_THROW_RUNTIME_ERROR(("raising error from " + source).c_str());
			}

		private:
			bool m_shouldRaiseError;
		};

		// endregion

		// region MockDifficultyChecker

		struct DifficultyCheckerParams {
		public:
			DifficultyCheckerParams(const std::vector<const model::Block*>& blocks, const cache::CatapultCache& cache)
					: Blocks(blocks)
					, Cache(cache)
			{}

		public:
			const std::vector<const model::Block*> Blocks;
			const cache::CatapultCache& Cache;
		};

		class MockDifficultyChecker : public test::ParamsCapture<DifficultyCheckerParams> {
		public:
			MockDifficultyChecker() : m_result(true)
			{}

		public:
			bool operator()(const std::vector<const model::Block*>& blocks, const cache::CatapultCache& cache) const {
				const_cast<MockDifficultyChecker*>(this)->push(blocks, cache);
				return m_result;
			}

		public:
			void setFailure() {
				m_result = false;
			}

		private:
			bool m_result;
		};

		// endregion

		// region MockUndoBlock

		struct UndoBlockParams {
		public:
			UndoBlockParams(const model::BlockElement& blockElement, observers::ObserverState& state, UndoBlockType undoBlockType)
					: pBlock(test::CopyEntity(blockElement.Block))
					, UndoBlockType(undoBlockType)
					, LastRecalculationHeight(state.Cache.dependentState().LastRecalculationHeight)
					, IsPassedMarkedCache(test::IsMarkedCache(state.Cache))
					, NumStatistics(state.Cache.sub<cache::BlockStatisticCache>().size())
			{}

		public:
			std::shared_ptr<const model::Block> pBlock;
			consumers::UndoBlockType UndoBlockType;
			const model::ImportanceHeight LastRecalculationHeight;
			const bool IsPassedMarkedCache;
			const size_t NumStatistics;
		};

		class MockUndoBlock : public test::ParamsCapture<UndoBlockParams> {
		public:
			void operator()(const model::BlockElement& blockElement, observers::ObserverState& state, UndoBlockType undoBlockType) const {
				const_cast<MockUndoBlock*>(this)->push(blockElement, state, undoBlockType);

				// simulate undoing a block by modifying the state to mark it
				// if the block is common, it should not change any state (but it is still checked in assertUnwind)
				if (UndoBlockType::Common == undoBlockType)
					return;

				auto& blockStatisticCache = state.Cache.sub<cache::BlockStatisticCache>();
				blockStatisticCache.insert(state::BlockStatistic(Height(blockStatisticCache.size() + 1)));

				auto& height = state.Cache.dependentState().LastRecalculationHeight;
				height = AddImportanceHeight(height, 1);
			}
		};

		// endregion

		// region MockProcessor

		struct ProcessorParams {
		public:
			ProcessorParams(const WeakBlockInfo& parentBlockInfo, const BlockElements& elements, observers::ObserverState& state)
					: pParentBlock(test::CopyEntity(parentBlockInfo.entity()))
					, ParentHash(parentBlockInfo.hash())
					, pElements(&elements)
					, LastRecalculationHeight(state.Cache.dependentState().LastRecalculationHeight)
					, IsPassedMarkedCache(test::IsMarkedCache(state.Cache))
					, NumStatistics(state.Cache.sub<cache::BlockStatisticCache>().size())
			{}

		public:
			std::shared_ptr<const model::Block> pParentBlock;
			const Hash256 ParentHash;
			const BlockElements* pElements;
			const model::ImportanceHeight LastRecalculationHeight;
			const bool IsPassedMarkedCache;
			const size_t NumStatistics;
		};

		class MockProcessor : public test::ParamsCapture<ProcessorParams> {
		public:
			MockProcessor() : m_result(ValidationResult::Success)
			{}

		public:
			ValidationResult operator()(
					const WeakBlockInfo& parentBlockInfo,
					BlockElements& elements,
					observers::ObserverState& state) const {
				const_cast<MockProcessor*>(this)->push(parentBlockInfo, elements, state);

				// mark the state by modifying it
				state.Cache.sub<cache::AccountStateCache>().addAccount(Sentinel_Processor_Public_Key, Height(1));
				state.Cache.dependentState().LastRecalculationHeight = Modified_Last_Recalculation_Height;

				// modify all the elements
				for (auto& element : elements)
					element.GenerationHash = { { static_cast<uint8_t>(element.Block.Height.unwrap()) } };

				return m_result;
			}

		public:
			void setResult(ValidationResult result) {
				m_result = result;
			}

		private:
			ValidationResult m_result;
		};

		// endregion

		// region MockStateChange

		struct StateChangeParams {
		public:
			StateChangeParams(const subscribers::StateChangeInfo& changeInfo)
					: ScoreDelta(changeInfo.ScoreDelta)
					// all processing should have occurred before the state change notification,
					// so the sentinel account should have been added
					, IsPassedProcessedCache(HasMarkedChanges(changeInfo.CacheChanges))
					, Height(changeInfo.Height)
			{}

		public:
			model::ChainScore ScoreDelta;
			bool IsPassedProcessedCache;
			catapult::Height Height;

		private:
			static bool HasMarkedChanges(const cache::CacheChanges& changes) {
				auto addedAccountStates = changes.sub<cache::AccountStateCache>().addedElements();
				return std::any_of(addedAccountStates.cbegin(), addedAccountStates.cend(), [](const auto* pAccountState) {
					return Sentinel_Processor_Public_Key == pAccountState->PublicKey;
				});
			}
		};

		class MockStateChange : public test::ParamsCapture<StateChangeParams>, public RaisableErrorSource {
		public:
			void operator()(const subscribers::StateChangeInfo& changeInfo) const {
				raise("MockStateChange");
				const_cast<MockStateChange*>(this)->push(changeInfo);
			}
		};

		// endregion

		// region MockPreStateWritten

		struct PreStateWrittenParams {
		public:
			PreStateWrittenParams(const cache::CatapultCacheDelta& cacheDelta, Height height)
					// all processing should have occurred before the pre state written notification,
					// so the sentinel account should have been added
					: IsPassedProcessedCache(cacheDelta.sub<cache::AccountStateCache>().contains(Sentinel_Processor_Public_Key))
					, BlockStatisticCachePruningBoundary(cacheDelta.sub<cache::BlockStatisticCache>().pruningBoundary())
					, LastRecalculationHeight(cacheDelta.dependentState().LastRecalculationHeight)
					, Height(height)
			{}

		public:
			bool IsPassedProcessedCache;
			deltaset::PruningBoundary<state::BlockStatistic> BlockStatisticCachePruningBoundary;
			model::ImportanceHeight LastRecalculationHeight;
			catapult::Height Height;
		};

		class MockPreStateWritten : public test::ParamsCapture<PreStateWrittenParams>, public RaisableErrorSource {
		public:
			void operator()(const cache::CatapultCacheDelta& cacheDelta, Height height) const {
				raise("MockPreStateWritten");
				const_cast<MockPreStateWritten*>(this)->push(cacheDelta, height);
			}
		};

		// endregion

		// region MockTransactionsChange

		struct TransactionsChangeParams {
		public:
			TransactionsChangeParams(const HashSet& addedTransactionHashes, const HashSet& revertedTransactionHashes)
					: AddedTransactionHashes(addedTransactionHashes)
					, RevertedTransactionHashes(revertedTransactionHashes)
			{}

		public:
			const HashSet AddedTransactionHashes;
			const HashSet RevertedTransactionHashes;
		};

		class MockTransactionsChange : public test::ParamsCapture<TransactionsChangeParams> {
		public:
			void operator()(const TransactionsChangeInfo& changeInfo) const {
				TransactionsChangeParams params(
						CopyHashes(changeInfo.AddedTransactionHashes),
						CopyHashes(changeInfo.RevertedTransactionInfos));
				const_cast<MockTransactionsChange*>(this)->push(std::move(params));
			}

		private:
			static HashSet CopyHashes(const utils::HashPointerSet& hashPointers) {
				HashSet hashes;
				for (const auto* pHash : hashPointers)
					hashes.insert(*pHash);

				return hashes;
			}

			static HashSet CopyHashes(const std::vector<model::TransactionInfo>& transactionInfos) {
				HashSet hashes;
				for (const auto& transactionInfo : transactionInfos)
					hashes.insert(transactionInfo.EntityHash);

				return hashes;
			}
		};

		// endregion

		// region MockCommitStep

		class MockCommitStep : public test::ParamsCapture<CommitOperationStep> {
		public:
			void operator()(CommitOperationStep step) const {
				const_cast<MockCommitStep*>(this)->push(step);
			}
		};

		// endregion

		// region test utils

		void SetBlockHeight(model::Block& block, Height height) {
			block.Timestamp = Timestamp(height.unwrap() * 1000);
			block.Difficulty = Difficulty();
			block.Height = height;
		}

		std::vector<InputSource> GetAllInputSources() {
			return { InputSource::Unknown, InputSource::Local, InputSource::Remote_Pull, InputSource::Remote_Push };
		}

		void LogInputSource(InputSource source) {
			CATAPULT_LOG(debug) << "source " << source;
		}

		ConsumerInput CreateInput(Height startHeight, uint32_t numBlocks, InputSource source = InputSource::Remote_Pull) {
			auto input = test::CreateConsumerInputWithBlocks(numBlocks, source);
			auto nextHeight = startHeight;
			for (const auto& element : input.blocks()) {
				SetBlockHeight(const_cast<model::Block&>(element.Block), nextHeight);
				nextHeight = nextHeight + Height(1);
			}

			return input;
		}

		// endregion

		// region CatapultCacheFactory

		class CatapultCacheFactory {
		public:
			struct PruneIdentifiers {
				std::vector<Height> Heights;
				std::vector<Timestamp> Times;
			};

		public:
			static cache::CatapultCache Create(PruneIdentifiers& pruneIdentifiers) {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.VotingSetGrouping = 1;

				std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(3);
				test::CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
				subCaches[PruneAwareCacheSubCachePlugin::Id] = std::make_unique<PruneAwareCacheSubCachePlugin>(pruneIdentifiers);

				auto cache = cache::CatapultCache(std::move(subCaches));
				test::AddMarkerAccount(cache);
				return cache;
			}

		private:
			class PruneAwareSubCacheView : public test::UnsupportedSubCacheView {
			public:
				explicit PruneAwareSubCacheView(PruneIdentifiers& pruneIdentifiers) : m_pruneIdentifiers(pruneIdentifiers)
				{}

			public:
				void prune(Height height) override {
					m_pruneIdentifiers.Heights.push_back(height);
				}

				void prune(Timestamp time) override {
					m_pruneIdentifiers.Times.push_back(time);
				}

			private:
				PruneIdentifiers& m_pruneIdentifiers;
			};

			class PruneAwareCacheSubCachePlugin : public test::UnsupportedSubCachePlugin<PruneAwareCacheSubCachePlugin> {
			public:
				static constexpr size_t Id = 2;
				static constexpr auto Name = "PruneAwareCache";

			public:
				explicit PruneAwareCacheSubCachePlugin(PruneIdentifiers& pruneIdentifiers) : m_pruneIdentifiers(pruneIdentifiers)
				{}

			public:
				std::unique_ptr<const cache::SubCacheView> createView() const override {
					return std::make_unique<PruneAwareSubCacheView>(m_pruneIdentifiers);
				}

				std::unique_ptr<cache::SubCacheView> createDelta() override {
					return std::make_unique<PruneAwareSubCacheView>(m_pruneIdentifiers);
				}

				void commit() override
				{}

			private:
				PruneIdentifiers& m_pruneIdentifiers;
			};
		};

		// endregion

		// region ConsumerTestContext

		struct ConsumerTestContext {
		public:
			ConsumerTestContext()
					: ConsumerTestContext(
							std::make_unique<mocks::MockMemoryBlockStorage>(),
							std::make_unique<mocks::MockMemoryBlockStorage>())
			{}

			ConsumerTestContext(std::unique_ptr<io::BlockStorage>&& pStorage, std::unique_ptr<io::PrunableBlockStorage>&& pStagingStorage)
					: Cache(CatapultCacheFactory::Create(CachePruneIdentifiers))
					, Storage(std::move(pStorage), std::move(pStagingStorage))
					, LastFinalizedHeight(Height(1)) {
				{
					auto cacheDelta = Cache.createDelta();
					cacheDelta.dependentState().LastRecalculationHeight = Initial_Last_Recalculation_Height;
					Cache.commit(Height(1));
				}

				BlockChainSyncHandlers handlers;
				handlers.DifficultyChecker = [this](const auto& blocks, const auto& cache) {
					return DifficultyChecker(blocks, cache);
				};
				handlers.LocalFinalizedHeightSupplier = [this]() {
					return LastFinalizedHeight;
				};
				handlers.UndoBlock = [this](const auto& block, auto& state, auto undoBlockType) {
					return UndoBlock(block, state, undoBlockType);
				};
				handlers.Processor = [this](const auto& parentBlockInfo, auto& elements, auto& state) {
					return Processor(parentBlockInfo, elements, state);
				};
				handlers.StateChange = [this](const auto& changeInfo) {
					return StateChange(changeInfo);
				};
				handlers.PreStateWritten = [this](const auto& cacheDelta, auto height) {
					return PreStateWritten(cacheDelta, height);
				};
				handlers.TransactionsChange = [this](const auto& changeInfo) {
					return TransactionsChange(changeInfo);
				};
				handlers.CommitStep = [this](auto step) {
					return CommitStep(step);
				};

				Consumer = CreateBlockChainSyncConsumer(Cache, Storage, handlers);
			}

		public:
			CatapultCacheFactory::PruneIdentifiers CachePruneIdentifiers;
			cache::CatapultCache Cache;
			io::BlockStorageCache Storage;
			Height LastFinalizedHeight;
			std::vector<std::shared_ptr<model::Block>> OriginalBlocks; // original stored blocks (excluding nemesis)

			MockDifficultyChecker DifficultyChecker;
			MockUndoBlock UndoBlock;
			MockProcessor Processor;
			MockStateChange StateChange;
			MockPreStateWritten PreStateWritten;
			MockTransactionsChange TransactionsChange;
			MockCommitStep CommitStep;

			disruptor::DisruptorConsumer Consumer;

		public:
			void seedStorage(Height desiredHeight, size_t numTransactionsPerBlock = 0) {
				// Arrange:
				auto height = Storage.view().chainHeight();
				auto storageModifier = Storage.modifier();

				while (height < desiredHeight) {
					height = height + Height(1);

					auto transactions = test::GenerateRandomTransactions(numTransactionsPerBlock);
					auto pBlock = test::GenerateBlockWithTransactions(transactions);
					SetBlockHeight(*pBlock, height);

					// - seed with random tx hashes
					auto blockElement = test::BlockToBlockElement(*pBlock);
					for (auto& transactionElement : blockElement.Transactions)
						transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();

					storageModifier.saveBlock(blockElement);
					OriginalBlocks.push_back(std::move(pBlock));
				}

				storageModifier.commit();
			}

		public:
			void assertDifficultyCheckerInvocation(const ConsumerInput& input) {
				// Assert:
				ASSERT_EQ(1u, DifficultyChecker.params().size());
				auto difficultyParams = DifficultyChecker.params()[0];

				EXPECT_EQ(&Cache, &difficultyParams.Cache);
				ASSERT_EQ(input.blocks().size(), difficultyParams.Blocks.size());
				for (auto i = 0u; i < input.blocks().size(); ++i)
					EXPECT_EQ(&input.blocks()[i].Block, difficultyParams.Blocks[i]) << "block at " << i;
			}

			void assertUnwind(const std::vector<Height>& unwoundHeights) {
				// Assert:
				ASSERT_EQ(unwoundHeights.size(), UndoBlock.params().size());
				auto i = 0u;
				for (auto height : unwoundHeights) {
					const auto& undoBlockParams = UndoBlock.params()[i];
					auto expectedHeight = AddImportanceHeight(Initial_Last_Recalculation_Height, i);
					auto expectedUndoType = i == unwoundHeights.size() - 1 ? UndoBlockType::Common : UndoBlockType::Rollback;
					auto message = "undo at " + std::to_string(i);

					EXPECT_EQ(*OriginalBlocks[(height - Height(2)).unwrap()], *undoBlockParams.pBlock) << message;
					EXPECT_EQ(expectedUndoType, undoBlockParams.UndoBlockType) << message;
					EXPECT_EQ(expectedHeight, undoBlockParams.LastRecalculationHeight) << message;
					EXPECT_TRUE(undoBlockParams.IsPassedMarkedCache) << message;
					EXPECT_EQ(i, undoBlockParams.NumStatistics) << message;
					++i;
				}
			}

			void assertProcessorInvocation(const ConsumerInput& input, size_t numUnwoundBlocks = 0) {
				// Assert:
				ASSERT_EQ(1u, Processor.params().size());
				const auto& processorParams = Processor.params()[0];
				auto expectedHeight = AddImportanceHeight(Initial_Last_Recalculation_Height, numUnwoundBlocks);
				auto pCommonBlockElement = Storage.view().loadBlockElement(input.blocks()[0].Block.Height - Height(1));

				EXPECT_EQ(pCommonBlockElement->Block, *processorParams.pParentBlock);
				EXPECT_EQ(pCommonBlockElement->EntityHash, processorParams.ParentHash);
				EXPECT_EQ(&input.blocks(), processorParams.pElements);
				EXPECT_EQ(expectedHeight, processorParams.LastRecalculationHeight);
				EXPECT_TRUE(processorParams.IsPassedMarkedCache);
				EXPECT_EQ(numUnwoundBlocks, processorParams.NumStatistics);
			}

			void assertNoStorageChanges() {
				// Assert: all original blocks are present in the storage
				auto storageView = Storage.view();
				ASSERT_EQ(Height(OriginalBlocks.size()) + Height(1), storageView.chainHeight());
				for (const auto& pBlock : OriginalBlocks) {
					auto pStorageBlock = storageView.loadBlock(pBlock->Height);
					EXPECT_EQ(*pBlock, *pStorageBlock) << "at height " << pBlock->Height;
				}

				// - the cache was not committed
				EXPECT_FALSE(Cache.sub<cache::AccountStateCache>().createView()->contains(Sentinel_Processor_Public_Key));
				EXPECT_EQ(0u, Cache.sub<cache::BlockStatisticCache>().createView()->size());

				// - no state changes were announced
				EXPECT_EQ(0u, StateChange.params().size());
				EXPECT_EQ(0u, PreStateWritten.params().size());

				// - the state was not changed
				EXPECT_EQ(Initial_Last_Recalculation_Height, Cache.createView().dependentState().LastRecalculationHeight);

				// - no transaction changes were announced
				EXPECT_EQ(0u, TransactionsChange.params().size());

				// - no commit steps were announced
				EXPECT_EQ(0u, CommitStep.params().size());
			}

			void assertStored(const ConsumerInput& input, const model::ChainScore& expectedScoreDelta) {
				// Assert: all input blocks should be saved in the storage
				auto storageView = Storage.view();
				auto inputHeight = input.blocks()[0].Block.Height;
				auto chainHeight = storageView.chainHeight();
				ASSERT_EQ(inputHeight + Height(input.blocks().size() - 1), chainHeight);
				for (auto height = inputHeight; height <= chainHeight; height = height + Height(1)) {
					auto pStorageBlock = storageView.loadBlock(height);
					EXPECT_EQ(input.blocks()[(height - inputHeight).unwrap()].Block, *pStorageBlock) << "at height " << height;
				}

				// - non conflicting original blocks should still be in storage
				for (auto height = Height(2); height < inputHeight; height = height + Height(1)) {
					auto pStorageBlock = storageView.loadBlock(height);
					EXPECT_EQ(*OriginalBlocks[(height - Height(2)).unwrap()], *pStorageBlock) << "at height " << height;
				}

				// - the cache was committed (add 1 to OriginalBlocks.size() because it does not include the nemesis)
				EXPECT_TRUE(Cache.sub<cache::AccountStateCache>().createView()->contains(Sentinel_Processor_Public_Key));
				EXPECT_EQ(
						OriginalBlocks.size() + 1 - inputHeight.unwrap() + 1,
						Cache.sub<cache::BlockStatisticCache>().createView()->size());
				EXPECT_EQ(chainHeight, Cache.createView().height());

				// - state changes were announced
				ASSERT_EQ(1u, StateChange.params().size());
				const auto& stateChangeParams = StateChange.params()[0];
				EXPECT_EQ(expectedScoreDelta, stateChangeParams.ScoreDelta);
				EXPECT_TRUE(stateChangeParams.IsPassedProcessedCache);
				EXPECT_EQ(chainHeight, stateChangeParams.Height);

				// - pre state written checkpoint was announced
				ASSERT_EQ(1u, PreStateWritten.params().size());
				const auto& preStateWrittenParams = PreStateWritten.params()[0];
				EXPECT_TRUE(preStateWrittenParams.IsPassedProcessedCache);
				EXPECT_EQ(Modified_Last_Recalculation_Height, preStateWrittenParams.LastRecalculationHeight);
				EXPECT_EQ(chainHeight, preStateWrittenParams.Height);

				// - the state was actually changed
				EXPECT_EQ(Modified_Last_Recalculation_Height, Cache.createView().dependentState().LastRecalculationHeight);

				// - transaction changes were announced
				EXPECT_EQ(1u, TransactionsChange.params().size());

				// - commit steps were announced
				ASSERT_EQ(3u, CommitStep.params().size());
				EXPECT_EQ(CommitOperationStep::Blocks_Written, CommitStep.params()[0]);
				EXPECT_EQ(CommitOperationStep::State_Written, CommitStep.params()[1]);
				EXPECT_EQ(CommitOperationStep::All_Updated, CommitStep.params()[2]);
			}
		};

		// endregion
	}

	// region basic

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		ConsumerTestContext context;

		// Assert:
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	// endregion

	// region height check

	namespace {
		void AssertInvalidHeight(
				Height localHeight,
				Height remoteHeight,
				uint32_t numRemoteBlocks,
				InputSource source,
				ValidationResult expectedResult = Failure_Consumer_Remote_Chain_Unlinked,
				Height lastFinalizedHeight = Height(1)) {
			// Arrange:
			ConsumerTestContext context;
			context.seedStorage(localHeight);
			context.LastFinalizedHeight = lastFinalizedHeight;
			auto input = CreateInput(remoteHeight, numRemoteBlocks, source);

			// Act:
			auto result = context.Consumer(input);

			// Assert:
			test::AssertAborted(result, expectedResult, disruptor::ConsumerResultSeverity::Failure);
			EXPECT_EQ(0u, context.DifficultyChecker.params().size());
			EXPECT_EQ(0u, context.UndoBlock.params().size());
			EXPECT_EQ(0u, context.Processor.params().size());
			context.assertNoStorageChanges();
		}

		void AssertValidHeight(
				Height localHeight,
				Height remoteHeight,
				uint32_t numRemoteBlocks,
				InputSource source,
				Height lastFinalizedHeight = Height(1)) {
			// Arrange:
			ConsumerTestContext context;
			context.seedStorage(localHeight);
			context.LastFinalizedHeight = lastFinalizedHeight;
			auto input = CreateInput(remoteHeight, numRemoteBlocks, source);

			// Act:
			context.Consumer(input);

			// Assert: if the height is valid, the difficulty checker must have been called
			EXPECT_EQ(1u, context.DifficultyChecker.params().size());
		}
	}

	TEST(TEST_CLASS, RemoteChainWithHeightLessThanTwoIsRejected) {
		// Arrange:
		for (auto source : GetAllInputSources()) {
			LogInputSource(source);

			// Act + Assert:
			AssertInvalidHeight(Height(1), Height(0), 3, source);
			AssertInvalidHeight(Height(1), Height(1), 3, source);
		}
	}

	TEST(TEST_CLASS, RemoteChainWithHeightAtLeastTwoIsValid) {
		// Arrange:
		for (auto source : GetAllInputSources()) {
			LogInputSource(source);

			// Act + Assert:
			AssertValidHeight(Height(1), Height(2), 3, source);
			AssertValidHeight(Height(2), Height(3), 3, source);
		}
	}

	TEST(TEST_CLASS, RemoteChainWithHeightMoreThanOneGreaterThanLocalHeightIsRejected) {
		// Arrange:
		for (auto source : GetAllInputSources()) {
			LogInputSource(source);

			// Act + Assert:
			AssertInvalidHeight(Height(100), Height(102), 3, source);
			AssertInvalidHeight(Height(100), Height(200), 3, source);
		}
	}

	TEST(TEST_CLASS, RemoteChainWithHeightLessThanLocalHeightIsOnlyValidForRemotePullSource) {
		// Arrange:
		for (auto source : GetAllInputSources()) {
			LogInputSource(source);

			// Act + Assert:
			if (InputSource::Remote_Pull == source) {
				AssertValidHeight(Height(100), Height(99), 1, source);
				AssertValidHeight(Height(100), Height(90), 1, source);
			} else {
				AssertInvalidHeight(Height(100), Height(99), 1, source);
				AssertInvalidHeight(Height(100), Height(90), 1, source);
			}
		}
	}

	TEST(TEST_CLASS, RemoteChainWithHeightAtOrOneGreaterThanLocalHeightIsValidForAllSources) {
		// Arrange:
		for (auto source : GetAllInputSources()) {
			LogInputSource(source);

			// Act + Assert:
			AssertValidHeight(Height(100), Height(100), 1, source);
			AssertValidHeight(Height(100), Height(101), 1, source);
		}
	}

	// the following tests only make sense for Remote_Pull because it is the only source that allows multiple block rollbacks

	namespace {
		void AssertDeepUnwindAllowed(Height localHeight, Height maxRemoteHeight, Height lastFinalizedHeight) {
			// Arrange:
			for (auto remoteHeight : { maxRemoteHeight + Height(10), maxRemoteHeight + Height(1), maxRemoteHeight }) {
				CATAPULT_LOG(debug) << "remoteHeight = " << remoteHeight;

				// Assert:
				AssertValidHeight(localHeight, remoteHeight, 4, InputSource::Remote_Pull, lastFinalizedHeight);
			}
		}

		void AssertDeepUnwindNotAllowed(Height localHeight, Height maxRemoteHeight, Height lastFinalizedHeight) {
			// Arrange:
			auto expectedResult = Failure_Consumer_Remote_Chain_Too_Far_Behind;
			for (auto remoteHeight : { maxRemoteHeight - Height(1), maxRemoteHeight - Height(10) }) {
				CATAPULT_LOG(debug) << "remoteHeight = " << remoteHeight;

				// Assert:
				AssertInvalidHeight(localHeight, remoteHeight, 4, InputSource::Remote_Pull, expectedResult, lastFinalizedHeight);
			}
		}
	}

	TEST(TEST_CLASS, RemoteChainFromRemotePullSourceCanUnwindAllUnfinalizedBlocksWhenOnlyNemesisIsFinalized) {
		AssertDeepUnwindAllowed(Height(100), Height(2), Height(1));
	}

	TEST(TEST_CLASS, RemoteChainFromRemotePullSourceCanUnwindAllUnfinalizedBlocks) {
		AssertDeepUnwindAllowed(Height(100), Height(51), Height(50));
	}

	TEST(TEST_CLASS, RemoteChainFromRemotePullSourceCannotUnwindAnyFinalizedBlocks) {
		AssertDeepUnwindNotAllowed(Height(100), Height(50), Height(50));
		AssertDeepUnwindNotAllowed(Height(100), Height(30), Height(50));
	}

	// endregion

	// region difficulties check

	TEST(TEST_CLASS, RemoteChainWithIncorrectDifficultiesIsRejected) {
		// Arrange: trigger a difficulty check failure
		ConsumerTestContext context;
		context.seedStorage(Height(3));
		context.DifficultyChecker.setFailure();

		auto input = CreateInput(Height(4), 2);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Difficulties_Mismatch, disruptor::ConsumerResultSeverity::Failure);
		EXPECT_EQ(0u, context.UndoBlock.params().size());
		EXPECT_EQ(0u, context.Processor.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertNoStorageChanges();
	}

	// endregion

	// region chain score test

	TEST(TEST_CLASS, ChainWithSmallerScoreIsRejected) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 5-6
		//          (note that the test setup ensures scores are linearly correlated with number of blocks)
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(5), 2);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Score_Not_Better, disruptor::ConsumerResultSeverity::Failure);
		EXPECT_EQ(4u, context.UndoBlock.params().size());
		EXPECT_EQ(0u, context.Processor.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5), Height(4) });
		context.assertNoStorageChanges();
	}

	TEST(TEST_CLASS, ChainWithIdenticalScoreIsRejected) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 6-7
		//          (note that the test setup ensures scores are linearly correlated with number of blocks)
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(6), 2);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Score_Not_Better, disruptor::ConsumerResultSeverity::Failure);
		EXPECT_EQ(3u, context.UndoBlock.params().size());
		EXPECT_EQ(0u, context.Processor.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5) });
		context.assertNoStorageChanges();
	}

	// endregion

	// region processor check

	namespace {
		void AssertRemoteChainWithNonSuccessProcessorResultIsRejected(
				ValidationResult processorResult,
				disruptor::ConsumerResultSeverity expectedConsumerResultSeverity) {
			// Arrange: configure the processor to return a non-success result
			ConsumerTestContext context;
			context.seedStorage(Height(3));
			context.Processor.setResult(processorResult);

			auto input = CreateInput(Height(4), 2);

			// Act:
			auto result = context.Consumer(input);

			// Assert:
			test::AssertAborted(result, processorResult, expectedConsumerResultSeverity);
			EXPECT_EQ(0u, context.UndoBlock.params().size());
			context.assertDifficultyCheckerInvocation(input);
			context.assertProcessorInvocation(input);
			context.assertNoStorageChanges();
		}
	}

	TEST(TEST_CLASS, RemoteChainWithProcessorFailureIsRejected_Neutral) {
		AssertRemoteChainWithNonSuccessProcessorResultIsRejected(ValidationResult::Neutral, disruptor::ConsumerResultSeverity::Neutral);
	}

	TEST(TEST_CLASS, RemoteChainWithProcessorFailureIsRejected_Failure) {
		AssertRemoteChainWithNonSuccessProcessorResultIsRejected(ValidationResult::Failure, disruptor::ConsumerResultSeverity::Failure);
	}

	// endregion

	// region successful syncs

	TEST(TEST_CLASS, CanSyncCompatibleChains) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 8-11
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(8), 4);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(0u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertProcessorInvocation(input);
		context.assertStored(input, model::ChainScore(4 * (Base_Difficulty - 1)));
	}

	TEST(TEST_CLASS, CanSyncIncompatibleChains) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 5-8
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(5), 4);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(4u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5), Height(4) });
		context.assertProcessorInvocation(input, 3);
		context.assertStored(input, model::ChainScore(Base_Difficulty - 1));
	}

	TEST(TEST_CLASS, CanSyncIncompatibleChainsWithOnlyLastBlockDifferent) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 7-10
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(7), 4);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(2u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6) });
		context.assertProcessorInvocation(input, 1);
		context.assertStored(input, model::ChainScore(3 * (Base_Difficulty - 1)));
	}

	TEST(TEST_CLASS, CanSyncIncompatibleChainsWhereShorterRemoteChainHasHigherScore) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 5
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(5), 1);
		const_cast<model::Block&>(input.blocks()[0].Block).Difficulty = Difficulty(Base_Difficulty * 3);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(4u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5), Height(4) });
		context.assertProcessorInvocation(input, 3);
		context.assertStored(input, model::ChainScore(2));
	}

	// endregion

	// region transaction notification

	namespace {
		void AssertHashesAreEqual(const std::vector<Hash256>& expected, const HashSet& actual) {
			EXPECT_EQ(expected.size(), actual.size());

			auto i = 0u;
			for (const auto& hash : expected) {
				auto message = "hash at " + std::to_string(i++);
				EXPECT_CONTAINS_MESSAGE(actual, hash, message);
			}
		}

		class InputTransactionBuilder {
		public:
			explicit InputTransactionBuilder(ConsumerInput& input) : m_input(input)
			{}

		public:
			const std::vector<Hash256>& hashes() const {
				return m_addedHashes;
			}

		public:
			void addRandom(size_t elementIndex, size_t numTransactions) {
				for (auto i = 0u; i < numTransactions; ++i)
					add(elementIndex, test::GenerateRandomTransaction(), test::GenerateRandomByteArray<Hash256>());
			}

			void addFromStorage(size_t elementIndex, const io::BlockStorageCache& storage, Height height, size_t txIndex) {
				auto pBlockElement = storage.view().loadBlockElement(height);

				auto i = 0u;
				for (const auto& transactionElement : pBlockElement->Transactions) {
					if (i++ != txIndex)
						continue;

					add(elementIndex, test::CopyEntity(transactionElement.Transaction), transactionElement.EntityHash);
					break;
				}
			}

		private:
			void add(size_t elementIndex, const std::shared_ptr<model::Transaction>& pTransaction, const Hash256& hash) {
				auto transactionElement = model::TransactionElement(*pTransaction);
				transactionElement.EntityHash = hash;

				m_input.blocks()[elementIndex].Transactions.push_back(transactionElement);
				m_addedHashes.push_back(hash);
				m_transactions.push_back(pTransaction); // keep the transaction alive
			}

		private:
			ConsumerInput& m_input;
			std::vector<Hash256> m_addedHashes;
			std::vector<std::shared_ptr<model::Transaction>> m_transactions;
		};

		std::vector<Hash256> ExtractTransactionHashesFromStorage(
				const io::BlockStorageView& storage,
				Height startHeight,
				Height endHeight) {
			std::vector<Hash256> hashes;
			for (auto h = startHeight; h <= endHeight; h = h + Height(1)) {
				auto pBlockElement = storage.loadBlockElement(h);
				for (const auto& transactionElement : pBlockElement->Transactions)
					hashes.push_back(transactionElement.EntityHash);
			}

			return hashes;
		}
	}

	TEST(TEST_CLASS, CanSyncCompatibleChains_TransactionNotification) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 8-11
		ConsumerTestContext context;
		context.seedStorage(Height(7), 3);
		auto input = CreateInput(Height(8), 4);

		// - add transactions to the input
		InputTransactionBuilder builder(input);
		builder.addRandom(0, 1);
		builder.addRandom(2, 3);
		builder.addRandom(3, 2);

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(0u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertProcessorInvocation(input);
		context.assertStored(input, model::ChainScore(4 * (Base_Difficulty - 1)));

		// - the change notification had 6 added and 0 reverted
		ASSERT_EQ(1u, context.TransactionsChange.params().size());
		const auto& txChangeParams = context.TransactionsChange.params()[0];

		EXPECT_EQ(6u, txChangeParams.AddedTransactionHashes.size());
		AssertHashesAreEqual(builder.hashes(), txChangeParams.AddedTransactionHashes);

		EXPECT_TRUE(txChangeParams.RevertedTransactionHashes.empty());
	}

	TEST(TEST_CLASS, CanSyncIncompatibleChains_TransactionNotification) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 5-8
		ConsumerTestContext context;
		context.seedStorage(Height(7), 3);
		auto input = CreateInput(Height(5), 4);

		// - add transactions to the input
		InputTransactionBuilder builder(input);
		builder.addRandom(0, 1);
		builder.addRandom(2, 3);
		builder.addRandom(3, 2);

		// - extract original hashes from storage
		auto expectedRevertedHashes = ExtractTransactionHashesFromStorage(context.Storage.view(), Height(5), Height(7));

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(4u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5), Height(4) });
		context.assertProcessorInvocation(input, 3);
		context.assertStored(input, model::ChainScore(Base_Difficulty - 1));

		// - the change notification had 6 added and 9 reverted
		ASSERT_EQ(1u, context.TransactionsChange.params().size());
		const auto& txChangeParams = context.TransactionsChange.params()[0];

		EXPECT_EQ(6u, txChangeParams.AddedTransactionHashes.size());
		AssertHashesAreEqual(builder.hashes(), txChangeParams.AddedTransactionHashes);

		EXPECT_EQ(9u, txChangeParams.RevertedTransactionHashes.size());
		AssertHashesAreEqual(expectedRevertedHashes, txChangeParams.RevertedTransactionHashes);
	}

	TEST(TEST_CLASS, CanSyncIncompatibleChainsWithSharedTransacions_TransactionNotification) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 5-8
		ConsumerTestContext context;
		context.seedStorage(Height(7), 3);
		auto input = CreateInput(Height(5), 4);

		// - add transactions to the input
		InputTransactionBuilder builder(input);
		builder.addRandom(0, 1);
		builder.addRandom(2, 3);
		builder.addRandom(3, 2);
		builder.addFromStorage(2, context.Storage, Height(5), 2);
		builder.addFromStorage(0, context.Storage, Height(7), 1);

		// - extract original hashes from storage
		auto expectedRevertedHashes = ExtractTransactionHashesFromStorage(context.Storage.view(), Height(5), Height(7));
		expectedRevertedHashes.erase(expectedRevertedHashes.begin() + 2 * 3 + 1); // block 7 tx 2
		expectedRevertedHashes.erase(expectedRevertedHashes.begin() + 2); // block 5 tx 3

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(4u, context.UndoBlock.params().size());
		context.assertDifficultyCheckerInvocation(input);
		context.assertUnwind({ Height(7), Height(6), Height(5), Height(4) });
		context.assertProcessorInvocation(input, 3);
		context.assertStored(input, model::ChainScore(Base_Difficulty - 1));

		// - the change notification had 8 added and 7 reverted
		ASSERT_EQ(1u, context.TransactionsChange.params().size());
		const auto& txChangeParams = context.TransactionsChange.params()[0];

		EXPECT_EQ(8u, txChangeParams.AddedTransactionHashes.size());
		AssertHashesAreEqual(builder.hashes(), txChangeParams.AddedTransactionHashes);

		EXPECT_EQ(7u, txChangeParams.RevertedTransactionHashes.size());
		AssertHashesAreEqual(expectedRevertedHashes, txChangeParams.RevertedTransactionHashes);
	}

	// endregion

	// region element updates

	TEST(TEST_CLASS, AllowsUpdateOfInputElements) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 8-11
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(8), 4);

		// Sanity: clear all generation hashes
		for (auto& blockElement : input.blocks())
			blockElement.GenerationHash = {};

		// Act:
		auto result = context.Consumer(input);

		// Sanity:
		test::AssertContinued(result);

		// Assert: the input generation hashes were updated
		uint8_t i = 8;
		for (const auto& blockElement : input.blocks()) {
			GenerationHash expectedGenerationHash{ { i++ } };
			EXPECT_EQ(expectedGenerationHash, blockElement.GenerationHash) << "generation hash at " << i;
		}
	}

	// endregion

	// region step notification

	namespace {
		class ErrorAwareBlockStorage : public mocks::MockMemoryBlockStorage, public RaisableErrorSource {
		public:
			void dropBlocksAfter(Height height) override {
				MockMemoryBlockStorage::dropBlocksAfter(height);
				raise("ErrorAwareBlockStorage::dropBlocksAfter");
			}
		};
	}

	TEST(TEST_CLASS, CommitStepsAreCorrectWhenWritingBlocksFails) {
		// Arrange:
		auto pBlockStorage = std::make_unique<ErrorAwareBlockStorage>();
		auto* pBlockStorageRaw = pBlockStorage.get();

		ConsumerTestContext context(std::make_unique<ErrorAwareBlockStorage>(), std::move(pBlockStorage));
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(6), 4);

		// - simulate block writing failure
		pBlockStorageRaw->setError();

		// Act:
		EXPECT_THROW(context.Consumer(input), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(0u, context.CommitStep.params().size());
	}

	TEST(TEST_CLASS, CommitStepsAreCorrectWhenWritingStateFails) {
		// Arrange:
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(6), 4);

		// - simulate state writing failure
		context.PreStateWritten.setError();

		// Act:
		EXPECT_THROW(context.Consumer(input), catapult_runtime_error);

		// Assert:
		ASSERT_EQ(1u, context.CommitStep.params().size());
		EXPECT_EQ(CommitOperationStep::Blocks_Written, context.CommitStep.params()[0]);
	}

	TEST(TEST_CLASS, CommitStepsAreCorrectWhenUpdatingFails) {
		// Arrange:
		auto pBlockStorage = std::make_unique<ErrorAwareBlockStorage>();
		auto* pBlockStorageRaw = pBlockStorage.get();

		ConsumerTestContext context(std::move(pBlockStorage), std::make_unique<ErrorAwareBlockStorage>());
		context.seedStorage(Height(7));
		auto input = CreateInput(Height(6), 4);

		// - simulate block updating failure
		pBlockStorageRaw->setError();

		// Act:
		EXPECT_THROW(context.Consumer(input), catapult_runtime_error);

		// Assert:
		ASSERT_EQ(2u, context.CommitStep.params().size());
		EXPECT_EQ(CommitOperationStep::Blocks_Written, context.CommitStep.params()[0]);
		EXPECT_EQ(CommitOperationStep::State_Written, context.CommitStep.params()[1]);
	}

	// endregion

	// region pruning

	namespace {
		void SetFinalizedHeightInDependentState(cache::CatapultCache& cache, Height height) {
			auto delta = cache.createDelta();
			delta.dependentState().LastFinalizedHeight = height;
			cache.commit(height);
		}
	}

	TEST(TEST_CLASS, CommitAutomaticallyPrunesCache) {
		// Arrange: create a local storage with blocks 1-7 and a remote storage with blocks 8-11
		ConsumerTestContext context;
		context.seedStorage(Height(7));
		context.LastFinalizedHeight = Height(5);
		auto input = CreateInput(Height(8), 4);

		SetFinalizedHeightInDependentState(context.Cache, Height(3));

		// - seed the BlockStatisticCache with entries up to local storage height
		{
			auto cacheDelta = context.Cache.createDelta();
			auto& blockStatisticCacheDelta = cacheDelta.sub<cache::BlockStatisticCache>();
			for (auto height = Height(1); height <= Height(7); height = height + Height(1))
				blockStatisticCacheDelta.insert(state::BlockStatistic(height));

			context.Cache.commit(Height(1));
		}

		// Sanity:
		EXPECT_EQ(Height(3), context.Cache.createView().dependentState().LastFinalizedHeight);
		EXPECT_EQ(7u, context.Cache.sub<cache::BlockStatisticCache>().createView()->size());

		// Act:
		auto result = context.Consumer(input);

		// Assert:
		test::AssertContinued(result);
		EXPECT_EQ(Height(5), context.Cache.createView().dependentState().LastFinalizedHeight);

		// - pruning was triggered on the cache (lower_bound should cause heights less than 5 to be pruned, so { 5, 6, 7 } are preserved)
		// - 7 (seeded entries); 0 (added entries, no real observer used); 5 (local finalized height)
		EXPECT_EQ(3u, context.Cache.sub<cache::BlockStatisticCache>().createView()->size());

		// - prune was called with expected identifiers
		EXPECT_EQ(std::vector<Height>({ Height(4), Height(5) }), context.CachePruneIdentifiers.Heights);
		EXPECT_EQ(std::vector<Timestamp>({ Timestamp(5000) }), context.CachePruneIdentifiers.Times);

		// - prune was called before state change notifications
		ASSERT_EQ(1u, context.PreStateWritten.params().size());
		EXPECT_EQ(Height(5), context.PreStateWritten.params()[0].BlockStatisticCachePruningBoundary.value().Height);
	}

	// endregion
}}
