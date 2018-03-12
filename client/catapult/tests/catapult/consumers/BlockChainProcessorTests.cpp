#include "catapult/consumers/BlockChainProcessor.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/consumers/BlockChainProcessorResults.h"
#include "catapult/consumers/InputUtils.h"
#include "catapult/model/BlockUtils.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;
using catapult::disruptor::BlockElements;

namespace catapult { namespace consumers {

#define TEST_CLASS BlockChainProcessorTests

	namespace {
		// region MockBlockHitPredicate

		struct BlockHitPredicateParams {
		public:
			BlockHitPredicateParams(const model::Block* pParent, const model::Block* pChild, const Hash256& generationHash)
					: pParentBlock(pParent)
					, pChildBlock(pChild)
					, GenerationHash(generationHash)
			{}

		public:
			const model::Block* pParentBlock;
			const model::Block* pChildBlock;
			const Hash256 GenerationHash;
		};

		class MockBlockHitPredicate : public test::ParamsCapture<BlockHitPredicateParams> {
		public:
			MockBlockHitPredicate() : m_numCalls(0), m_trigger(std::numeric_limits<size_t>::max())
			{}

		public:
			bool operator()(const model::Block& parent, const model::Block& child, const Hash256& generationHash) const {
				const_cast<MockBlockHitPredicate*>(this)->push(&parent, &child, generationHash);
				return ++m_numCalls < m_trigger;
			}

		public:
			void setFailure(size_t trigger) {
				m_trigger = trigger;
			}

		private:
			mutable size_t m_numCalls;
			size_t m_trigger;
		};

		// endregion

		// region MockBlockHitPredicateFactory

		struct BlockHitPredicateFactoryParams {
		public:
			BlockHitPredicateFactoryParams(const cache::ReadOnlyCatapultCache& cache)
					: IsPassedMarkedCache(test::IsMarkedCache(cache))
					, NumDifficultyInfos(cache.sub<cache::BlockDifficultyCache>().size())
			{}

		public:
			const bool IsPassedMarkedCache;
			const size_t NumDifficultyInfos;
		};

		class MockBlockHitPredicateFactory : public test::ParamsCapture<BlockHitPredicateFactoryParams> {
		public:
			MockBlockHitPredicateFactory(const MockBlockHitPredicate& blockHitPredicate)
					: m_blockHitPredicate(blockHitPredicate)
			{}

		public:
			BlockHitPredicate operator()(const cache::ReadOnlyCatapultCache& cache) const {
				const_cast<MockBlockHitPredicateFactory*>(this)->push(cache);

				const auto& blockHitPredicate = m_blockHitPredicate;
				return [&blockHitPredicate](const auto& parent, const auto& child, const auto& generationHash) {
					return blockHitPredicate(parent, child, generationHash);
				};
			}

		private:
			const MockBlockHitPredicate& m_blockHitPredicate;
		};

		// endregion

		// region MockBatchEntityProcessor

		struct BatchEntityProcessorParams {
		public:
			BatchEntityProcessorParams(
					catapult::Height height,
					catapult::Timestamp timestamp,
					const model::WeakEntityInfos& entityInfos,
					const observers::ObserverState& state)
					: Height(height)
					, Timestamp(timestamp)
					, EntityInfos(entityInfos)
					, pState(&state.State)
					, IsPassedMarkedCache(test::IsMarkedCache(state.Cache))
					, NumDifficultyInfos(state.Cache.sub<cache::BlockDifficultyCache>().size())
			{}

		public:
			const catapult::Height Height;
			const catapult::Timestamp Timestamp;
			const model::WeakEntityInfos EntityInfos;
			const state::CatapultState* pState;
			const bool IsPassedMarkedCache;
			const size_t NumDifficultyInfos;
		};

		class MockBatchEntityProcessor : public test::ParamsCapture<BatchEntityProcessorParams> {
		public:
			MockBatchEntityProcessor() : m_numCalls(0), m_trigger(0), m_result(ValidationResult::Success)
			{}

		public:
			ValidationResult operator()(
					Height height,
					Timestamp timestamp,
					const model::WeakEntityInfos& entityInfos,
					const observers::ObserverState& state) const {
				const_cast<MockBatchEntityProcessor*>(this)->push(height, timestamp, entityInfos, state);

				// - add a block difficulty info to the cache as a marker
				auto& blockDifficultyCache = state.Cache.sub<cache::BlockDifficultyCache>();
				blockDifficultyCache.insert(state::BlockDifficultyInfo(Height(blockDifficultyCache.size() + 1)));

				return ++m_numCalls < m_trigger ? ValidationResult::Success : m_result;
			}

		public:
			void setResult(ValidationResult result, size_t trigger) {
				m_result = result;
				m_trigger = trigger;
			}

		private:
			mutable size_t m_numCalls;
			size_t m_trigger;
			ValidationResult m_result;
		};

		// endregion

		model::WeakEntityInfos ExtractEntityInfosFromBlock(const model::BlockElement& element) {
			model::WeakEntityInfos entityInfos;
			ExtractEntityInfos(element, entityInfos);
			return entityInfos;
		}

		struct ProcessorTestContext {
		public:
			ProcessorTestContext() : BlockHitPredicateFactory(BlockHitPredicate) {
				Processor = CreateBlockChainProcessor(
						[this](const auto& cache) {
							return BlockHitPredicateFactory(cache);
						},
						[this](auto height, auto timestamp, const auto& entities, const auto& state) {
							return BatchEntityProcessor(height, timestamp, entities, state);
						});
			}

		public:
			state::CatapultState State;

			MockBlockHitPredicate BlockHitPredicate;
			MockBlockHitPredicateFactory BlockHitPredicateFactory;
			MockBatchEntityProcessor BatchEntityProcessor;
			BlockChainProcessor Processor;

		public:
			ValidationResult Process(const model::BlockElement& parentBlockElement, BlockElements& elements) {
				auto cache = test::CreateCatapultCacheWithMarkerAccount();
				auto delta = cache.createDelta();

				return Processor(WeakBlockInfo(parentBlockElement), elements, observers::ObserverState(delta, State));
			}

			ValidationResult Process(const model::Block& parentBlock, BlockElements& elements) {
				// use the real parent block hash to ensure the chain is linked
				return Process(test::BlockToBlockElement(parentBlock), elements);
			}

		public:
			void assertBlockHitPredicateCalls(const model::Block& parentBlock, const BlockElements& elements) {
				// block hit predicate factory
				{
					const auto& allParams = BlockHitPredicateFactory.params();
					EXPECT_EQ(1u, allParams.size());

					auto i = 0u;
					for (auto params : allParams) {
						auto message = "hit predicate factory at " + std::to_string(i);
						EXPECT_TRUE(params.IsPassedMarkedCache) << message;
						EXPECT_EQ(i, params.NumDifficultyInfos) << message;
						++i;
					}
				}

				// block hit
				{
					const auto& allParams = BlockHitPredicate.params();
					ASSERT_LE(allParams.size(), elements.size());

					auto i = 0u;
					const auto* pPreviousBlock = &parentBlock;
					for (auto params : allParams) {
						auto message = "hit predicate at " + std::to_string(i);
						const auto& currentBlockElement = elements[i];
						const auto* pCurrentBlock = &currentBlockElement.Block;
						EXPECT_EQ(pPreviousBlock, params.pParentBlock) << message;
						EXPECT_EQ(pCurrentBlock, params.pChildBlock) << message;
						EXPECT_EQ(currentBlockElement.GenerationHash, params.GenerationHash) << message;
						pPreviousBlock = pCurrentBlock;
						++i;
					}
				}
			}

			void assertBatchEntityProcessorCalls(const BlockElements& elements) {
				const auto& allParams = BatchEntityProcessor.params();
				ASSERT_LE(allParams.size(), elements.size());

				auto i = 0u;
				for (auto params : allParams) {
					auto message = "processor at " + std::to_string(i);
					const auto& blockElement = elements[i];
					auto expectedEntityInfos = ExtractEntityInfosFromBlock(blockElement);

					EXPECT_EQ(blockElement.Block.Height, params.Height) << message;
					EXPECT_EQ(blockElement.Block.Timestamp, params.Timestamp) << message;
					EXPECT_EQ(expectedEntityInfos, params.EntityInfos) << message;
					EXPECT_EQ(&State, params.pState) << message;
					EXPECT_TRUE(params.IsPassedMarkedCache) << message;
					EXPECT_EQ(i, params.NumDifficultyInfos) << message;
					++i;
				}
			}

			void assertNoHandlerCalls() {
				EXPECT_EQ(0u, BlockHitPredicate.params().size());
				EXPECT_EQ(0u, BlockHitPredicateFactory.params().size());
				EXPECT_EQ(0u, BatchEntityProcessor.params().size());
			}
		};
	}

	TEST(TEST_CLASS, EmptyInputResultsInNeutralResult) {
		// Arrange:
		ProcessorTestContext context;
		auto pParentBlock = test::GenerateEmptyRandomBlock();
		BlockElements elements;

		// Act:
		auto result = context.Process(*pParentBlock, elements);

		// Assert:
		EXPECT_EQ(ValidationResult::Neutral, result);
		context.assertNoHandlerCalls();
	}

	namespace {
		void AssertUnlinkedChain(const consumer<model::Block&>& unlink) {
			ProcessorTestContext context;
			auto pParentBlock = test::GenerateEmptyRandomBlock();
			pParentBlock->Height = Height(11);
			auto elements = test::CreateBlockElements(1);
			test::LinkBlocks(*pParentBlock, const_cast<model::Block&>(elements[0].Block));
			unlink(const_cast<model::Block&>(elements[0].Block));

			// Act:
			auto result = context.Process(*pParentBlock, elements);

			// Assert:
			EXPECT_EQ(Failure_Chain_Unlinked, result);
			context.assertNoHandlerCalls();
		}
	}

	TEST(TEST_CLASS, ChainPartHeightMustLinkToParent) {
		// Assert: invalidate the height
		AssertUnlinkedChain([](auto& block) { block.Height = block.Height + Height(1); });
	}

	TEST(TEST_CLASS, ChainPartPreviousBlockHashMustLinkToParent) {
		// Assert: invalidate the previous block hash
		AssertUnlinkedChain([](auto& block) { ++block.PreviousBlockHash[0]; });
	}

	namespace {
		void AssertCanProcessValidElements(BlockElements& elements, size_t numExpectedBlocks) {
			ProcessorTestContext context;
			auto pParentBlock = test::GenerateEmptyRandomBlock();
			pParentBlock->Height = Height(11);
			test::LinkBlocks(*pParentBlock, const_cast<model::Block&>(elements[0].Block));

			// Act:
			auto result = context.Process(*pParentBlock, elements);

			// Assert:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(numExpectedBlocks, context.BlockHitPredicate.params().size());
			EXPECT_EQ(numExpectedBlocks, context.BatchEntityProcessor.params().size());
			context.assertBlockHitPredicateCalls(*pParentBlock, elements);
			context.assertBatchEntityProcessorCalls(elements);
		}
	}

	TEST(TEST_CLASS, CanProcessSingleBlockWithoutTransactions) {
		// Arrange:
		auto elements = test::CreateBlockElements(1);

		// Assert:
		AssertCanProcessValidElements(elements, 1);
	}

	TEST(TEST_CLASS, CanProcessSingleBlockWithTransactions) {
		// Arrange:
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(3, 12);
		auto elements = test::CreateBlockElements({ pBlock.get() });

		// Assert:
		AssertCanProcessValidElements(elements, 1);
	}

	TEST(TEST_CLASS, CanProcessMultipleBlocksWithoutTransactionss) {
		// Arrange:
		auto elements = test::CreateBlockElements(3);

		// Assert:
		AssertCanProcessValidElements(elements, 3);
	}

	TEST(TEST_CLASS, CanProcessMultipleBlocksWithTransactions) {
		// Arrange:
		auto pBlock1 = test::GenerateBlockWithTransactionsAtHeight(3, 12);
		auto pBlock2 = test::GenerateBlockWithTransactionsAtHeight(2, 13);
		auto pBlock3 = test::GenerateBlockWithTransactionsAtHeight(4, 14);
		auto elements = test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get() });

		// Assert:
		AssertCanProcessValidElements(elements, 3);
	}

	TEST(TEST_CLASS, ExecuteShortCircutsOnUnhitBlock) {
		// Arrange: cause the second hit check to return false
		ProcessorTestContext context;
		context.BlockHitPredicate.setFailure(2);

		auto pParentBlock = test::GenerateEmptyRandomBlock();
		pParentBlock->Height = Height(11);
		auto elements = test::CreateBlockElements(3);
		test::LinkBlocks(*pParentBlock, const_cast<model::Block&>(elements[0].Block));

		// Act:
		auto result = context.Process(*pParentBlock, elements);

		// Assert:
		// - block hit predicate returned { true, false }
		// - only one processor was called (after true, but not false)
		EXPECT_EQ(Failure_Chain_Block_Not_Hit, result);
		EXPECT_EQ(2u, context.BlockHitPredicate.params().size());
		EXPECT_EQ(1u, context.BatchEntityProcessor.params().size());
		context.assertBlockHitPredicateCalls(*pParentBlock, elements);
		context.assertBatchEntityProcessorCalls(elements);
	}

	namespace {
		void AssertShortCircutsOnProcessorResult(ValidationResult processorResult) {
			// Arrange: cause the second processor call to return a non-success code
			ProcessorTestContext context;
			context.BatchEntityProcessor.setResult(processorResult, 2);

			auto pParentBlock = test::GenerateEmptyRandomBlock();
			pParentBlock->Height = Height(11);
			auto elements = test::CreateBlockElements(3);
			test::LinkBlocks(*pParentBlock, const_cast<model::Block&>(elements[0].Block));

			// Act:
			auto result = context.Process(*pParentBlock, elements);

			// Assert:
			// - block hit predicate returned true
			// - the second processor returned a non-success code
			EXPECT_EQ(processorResult, result);
			EXPECT_EQ(2u, context.BlockHitPredicate.params().size());
			EXPECT_EQ(2u, context.BatchEntityProcessor.params().size());
			context.assertBlockHitPredicateCalls(*pParentBlock, elements);
			context.assertBatchEntityProcessorCalls(elements);
		}
	}

	TEST(TEST_CLASS, ExecuteShortCircutsOnProcessorResult_Neutral) {
		// Assert:
		AssertShortCircutsOnProcessorResult(ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, ExecuteShortCircutsOnProcessorResult_Failure) {
		// Assert:
		AssertShortCircutsOnProcessorResult(ValidationResult::Failure);
	}

	// region generation hashes update

	namespace {
		void AssertGenerationHashesAreUpdatedCorrectly(BlockElements& elements, size_t numExpectedBlocks) {
			// Arrange:
			ProcessorTestContext context;
			auto pParentBlock = test::GenerateEmptyRandomBlock();
			pParentBlock->Height = Height(11);
			test::LinkBlocks(*pParentBlock, const_cast<model::Block&>(elements[0].Block));

			// - clear all generation hashes
			for (auto& blockElement : elements)
				blockElement.GenerationHash = {};

			// Act:
			auto parentBlockElement = test::BlockToBlockElement(*pParentBlock);
			test::FillWithRandomData(parentBlockElement.GenerationHash);
			auto result = context.Process(parentBlockElement, elements);

			// Sanity:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(numExpectedBlocks, elements.size());

			// Assert: generation hashes were calculated
			auto i = 0u;
			auto previousGenerationHash = parentBlockElement.GenerationHash;
			for (const auto& blockElement : elements) {
				auto expectedGenerationHash = model::CalculateGenerationHash(previousGenerationHash, blockElement.Block.Signer);
				EXPECT_EQ(expectedGenerationHash, blockElement.GenerationHash) << "generation hash at " << i++;
				previousGenerationHash = expectedGenerationHash;
			}
		}
	}

	TEST(TEST_CLASS, SetsGenerationHashesInSingleBlockInput) {
		// Arrange:
		auto elements = test::CreateBlockElements(1);

		// Assert:
		AssertGenerationHashesAreUpdatedCorrectly(elements, 1);
	}

	TEST(TEST_CLASS, SetsGenerationHashesInMultiBlockInput) {
		// Arrange:
		auto elements = test::CreateBlockElements(3);

		// Assert:
		AssertGenerationHashesAreUpdatedCorrectly(elements, 3);
	}

	// endregion
}}
