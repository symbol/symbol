#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/chain/RemoteApi.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/ChainScore.h"
#include "catapult/model/EntityRange.h"
#include "tests/catapult/chain/utils/MockChainApi.h"
#include "tests/catapult/chain/utils/MockTransactionApi.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

using namespace catapult::model;
using catapult::mocks::MockPacketIo;
using catapult::mocks::MockChainApi;
using catapult::mocks::MockTransactionApi;

namespace catapult { namespace chain {

	namespace {
		constexpr Height Default_Height(20);

		disruptor::ConsumerCompletionResult CreateContinueResult() {
			disruptor::ConsumerCompletionResult result;
			result.CompletionStatus = disruptor::CompletionStatus::Normal;
			return result;
		}

		disruptor::ConsumerCompletionResult CreateAbortResult() {
			disruptor::ConsumerCompletionResult result;
			result.CompletionStatus = disruptor::CompletionStatus::Aborted;
			return result;
		}

		ChainSynchronizerConfiguration CreateConfiguration() {
			auto config = ChainSynchronizerConfiguration();
			config.MaxBlocksPerSyncAttempt = 4 * 100;
			config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512).bytes32();
			config.MaxRollbackBlocks = 360;
			return config;
		}

		struct TestContext {
			TestContext(const ChainScore& localScore, const ChainScore& remoteScore)
					: TestContext(localScore, remoteScore, {}, {}, test::GenerateVerifiableBlockAtHeight(Default_Height), {}, {})
			{}

			TestContext(const ChainScore& localScore, const ChainScore& remoteScore, std::unique_ptr<Block>&& pRemoteLastBlock)
					: TestContext(localScore, remoteScore, {}, {}, std::move(pRemoteLastBlock), {}, {})
			{}

			TestContext(
					const ChainScore& localScore,
					const ChainScore& remoteScore,
					const ShortHashRange& localShortHashes,
					const TransactionRange& remoteTransactions)
					: TestContext(
							localScore,
							remoteScore,
							{},
							{},
							test::GenerateVerifiableBlockAtHeight(Default_Height),
							localShortHashes,
							remoteTransactions)
			{}

			TestContext(
					const ChainScore& localScore,
					const ChainScore& remoteScore,
					const HashRange& localHashes,
					const HashRange& remoteHashes,
					std::unique_ptr<Block>&& pRemoteLastBlock)
					: TestContext(localScore, remoteScore, localHashes, remoteHashes, std::move(pRemoteLastBlock), {}, {})
			{}

			TestContext(
					const ChainScore& localScore,
					const ChainScore& remoteScore,
					const HashRange& localHashes,
					const HashRange& remoteHashes,
					std::unique_ptr<Block>&& pRemoteLastBlock,
					const ShortHashRange& localShortHashes,
					const TransactionRange& remoteTransactions)
					: LocalScore(localScore)
					, LocalHashes(HashRange::CopyRange(localHashes))
					, pIo(std::make_shared<MockPacketIo>())
					, pChainApi(std::make_shared<MockChainApi>(remoteScore, std::move(pRemoteLastBlock), remoteHashes))
					, pTransactionApi(std::make_shared<MockTransactionApi>(remoteTransactions))
					, BlockRangeConsumerCalls(0)
					, TransactionRangeConsumerCalls(0)
					, ShortHashSupplierCalls(0)
					, ShortHashes(ShortHashRange::CopyRange(localShortHashes))
					, Config(CreateConfiguration()) {
				Remote.pChainApi = pChainApi;
				Remote.pTransactionApi = pTransactionApi;
			}

			void assertNoCalls() const {
				EXPECT_EQ(0u, ShortHashSupplierCalls);
				EXPECT_EQ(0u, TransactionRangeConsumerCalls);
				EXPECT_EQ(0u, BlockRangeConsumerCalls);
			}

			ChainScore LocalScore;
			HashRange LocalHashes;
			std::shared_ptr<MockPacketIo> pIo;
			std::shared_ptr<MockChainApi> pChainApi;
			std::shared_ptr<MockTransactionApi> pTransactionApi;
			RemoteApi Remote;
			size_t BlockRangeConsumerCalls;
			size_t TransactionRangeConsumerCalls;
			size_t ShortHashSupplierCalls;
			ShortHashRange ShortHashes;
			ChainSynchronizerConfiguration Config;
			TransactionRange ConsumerTransactionRange;
			disruptor::ProcessingCompleteFunc ProcessingComplete;
		};

		ChainSynchronizer CreateSynchronizer(TestContext& context) {
			auto pLocal = std::make_shared<MockChainApi>(
					context.LocalScore,
					test::GenerateVerifiableBlockAtHeight(Default_Height),
					context.LocalHashes);

			auto& shortHashSupplierCalls = context.ShortHashSupplierCalls;
			const auto& shortHashRange = context.ShortHashes;
			auto shortHashesSupplier = [&shortHashSupplierCalls, &shortHashRange]() {
				++shortHashSupplierCalls;
				return ShortHashRange::CopyRange(shortHashRange);
			};

			auto& blockConsumerCalls = context.BlockRangeConsumerCalls;
			auto blockRangeConsumer = [&blockConsumerCalls, &context](const auto&, const auto& processingComplete) {
				++blockConsumerCalls;
				context.ProcessingComplete = processingComplete;
				return blockConsumerCalls;
			};

			auto& txConsumerCalls = context.TransactionRangeConsumerCalls;
			auto& txRange = context.ConsumerTransactionRange;
			auto transactionRangeConsumer = [&txConsumerCalls, &txRange](auto&& transactionRange) {
				++txConsumerCalls;
				txRange = std::move(transactionRange);
			};

			return CreateChainSynchronizer(
					pLocal,
					context.Config,
					shortHashesSupplier,
					blockRangeConsumer,
					transactionRangeConsumer);
		}

		void AssertSync(const TestContext& context, size_t numBlockConsumerCalls) {
			EXPECT_EQ(0u, context.ShortHashSupplierCalls);
			EXPECT_EQ(0u, context.TransactionRangeConsumerCalls);
			EXPECT_EQ(numBlockConsumerCalls, context.BlockRangeConsumerCalls);
		}
	}

	// region chain synchronization

	namespace {
		void AssertNeutralInteraction(ChainScore localScore, ChainScore remoteScore, size_t numExpectedUnconfirmedCalls) {
			// Arrange:
			TestContext context(localScore, remoteScore);
			auto synchronizer = CreateSynchronizer(context);

			// Act:
			auto result = synchronizer(context.Remote).get();

			// Assert:
			EXPECT_EQ(NodeInteractionResult::Neutral, result);
			EXPECT_EQ(numExpectedUnconfirmedCalls, context.ShortHashSupplierCalls);
			EXPECT_EQ(numExpectedUnconfirmedCalls, context.TransactionRangeConsumerCalls);
			EXPECT_EQ(0u, context.BlockRangeConsumerCalls);
		}
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfCompareChainsReturnsEqualScoreResult) {
		// Assert:
		AssertNeutralInteraction(ChainScore(10), ChainScore(10), 1);
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfCompareChainsReturnsLowerScoreResult) {
		// Assert:
		AssertNeutralInteraction(ChainScore(11), ChainScore(10), 0);
	}

	TEST(ChainSynchronizerTests, FailedInteractionIfCompareChainsReturnsFailureResult) {
		// Arrange: trigger failure by letting the remote return a non-verifiable last block
		TestContext context(ChainScore(10), ChainScore(11), test::GenerateNonVerifiableBlockAtHeight(Default_Height));
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
	}

	TEST(ChainSynchronizerTests, FailedInteractionIfCompareChainsReturnsException) {
		// Arrange:
		TestContext context(ChainScore(10), ChainScore(11));
		context.pChainApi->setError(MockChainApi::EntryPoint::Chain_Info);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
	}

	namespace {
		auto CreateDefaultTestContext(size_t numLocalHashes, size_t numRemoteHashes) {
			auto remoteHashes = test::GenerateRandomHashes(numRemoteHashes);
			auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, numLocalHashes);
			TestContext context(
					ChainScore(10),
					ChainScore(11),
					localHashes,
					remoteHashes,
					test::GenerateVerifiableBlockAtHeight(Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			context.Config.MaxRollbackBlocks = 9;
			context.Config.MaxChainBytesPerSyncAttempt = 23;
			return context;
		}

		void AssertDefaultSinglePullRequest(const mocks::MockChainApi& chainApi) {
			// Assert:
			ASSERT_EQ(1u, chainApi.blocksFromRequests().size());
			const auto& params = chainApi.blocksFromRequests()[0];
			EXPECT_EQ(Default_Height, params.first); // localHeight - maxRollback + firstDifferenceIndex
			EXPECT_EQ(9, params.second.NumBlocks); // maxRollback
			EXPECT_EQ(23, params.second.NumBytes);
		}

		void AssertDefaultMultiplePullRequest(const mocks::MockChainApi& chainApi, const std::vector<Height>& expectedRequestHeights) {
			// Assert:
			ASSERT_EQ(expectedRequestHeights.size(), chainApi.blocksFromRequests().size());

			auto i = 0u;
			for (const auto& params : chainApi.blocksFromRequests()) {
				EXPECT_EQ(expectedRequestHeights[i], params.first) << "height of request " << i;
				EXPECT_EQ(9, params.second.NumBlocks) << "NumBlocks of request " << i; // maxRollback
				EXPECT_EQ(23, params.second.NumBytes) << "NumBytes of request " << i;
				++i;
			}
		}
	}

	TEST(ChainSynchronizerTests, SuccessfulInteractionIfRemoteChainPartWasSuccessfullyPulled) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(ChainSynchronizerTests, SuccessfulInteractionWithMultiplePulls) {
		// Arrange:
		// - last block has height 20, rewrite limit is 9
		// - common block has height 14 = 20 - 9 + 4 - 1
		// - pulls 2 blocks at time: 3 attempts needed to pull 6 blocks
		auto context = CreateDefaultTestContext(4, 10);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, { Height(15), Height(17), Height(19) });
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfRemoteDoesNotHaveBlocksAtRequestedHeight) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Neutral, result);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(ChainSynchronizerTests, SuccessfulInteractionWithMultiplePullsWhenRemoteRunsOutOfBlocks) {
		// Arrange:
		// - last block has height 20, rewrite limit is 9
		// - common block has height 14 = 20 - 9 + 4 - 1
		// - pulls 2 blocks at time: 3 attempts needed to pull 6 blocks but blocks are only returned 2/3 times
		auto context = CreateDefaultTestContext(4, 10);
		context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2, 2, 0 });
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Success, result);
		AssertSync(context, 1);
		AssertDefaultMultiplePullRequest(*context.pChainApi, { Height(15), Height(17), Height(19) });
	}

	TEST(ChainSynchronizerTests, FailedInteractionIfBlocksFromReturnsException) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		context.pChainApi->setError(MockChainApi::EntryPoint::Blocks_From);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		context.assertNoCalls();
		AssertDefaultSinglePullRequest(*context.pChainApi);
	}

	TEST(ChainSynchronizerTests, ReturnsNotReadyFutureWhenPullingBlocks) {
		// Arrange:
		auto context = CreateDefaultTestContext(9, 10);
		context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto future = synchronizer(context.Remote);

		// Assert:
		EXPECT_FALSE(future.is_ready());

		// Cleanup: wait for future to complete
		future.get();
	}

	// endregion

	// region pull transactions

	namespace {
		constexpr size_t Num_Short_Hashes = 5;

		ShortHashRange CreateShortHashRange() {
			auto buffer = test::GenerateRandomData<Num_Short_Hashes * sizeof(ShortHashRange)>();
			return ShortHashRange::CopyFixed(buffer.data(), Num_Short_Hashes);
		}

		template<typename TEntity>
		void AssertEqualRange(
				const EntityRange<TEntity>& expectedRange,
				const EntityRange<TEntity>& actualRange,
				const char* message) {
			ASSERT_EQ(expectedRange.size(), actualRange.size());
			auto iter = expectedRange.cbegin();
			auto i = 0u;
			for (const auto& entity : actualRange) {
				EXPECT_EQ(*iter++, entity) << message << " at " << i;
				++i;
			}
		}
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfTransactionsWereSuccessfullyPulled) {
		// Arrange:
		auto shortHashRange = CreateShortHashRange();
		auto transactionRange = test::CreateTransactionEntityRange(5);
		TestContext context(ChainScore(10), ChainScore(10), shortHashRange, transactionRange);
		auto synchronizer = CreateSynchronizer(context);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Neutral, result);
		EXPECT_EQ(1u, context.ShortHashSupplierCalls);
		EXPECT_EQ(1u, context.TransactionRangeConsumerCalls);
		ASSERT_EQ(1u, context.pTransactionApi->unconfirmedTransactionsRequests().size());
		EXPECT_EQ(0u, context.BlockRangeConsumerCalls);

		const auto& actualShortHashRange = context.pTransactionApi->unconfirmedTransactionsRequests()[0];
		AssertEqualRange(shortHashRange, actualShortHashRange, "short hash");

		AssertEqualRange(transactionRange, context.ConsumerTransactionRange, "transaction");
	}

	TEST(ChainSynchronizerTests, FailedInteractionIfUnconfirmedTransactionsReturnsException) {
		auto shortHashRange = CreateShortHashRange();
		auto transactionRange = test::CreateTransactionEntityRange(5);
		TestContext context(ChainScore(10), ChainScore(10), shortHashRange, transactionRange);
		auto synchronizer = CreateSynchronizer(context);
		context.pTransactionApi->setError(MockTransactionApi::EntryPoint::Unconfirmed_Transactions);

		// Act:
		auto result = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Failure, result);
		EXPECT_EQ(1u, context.ShortHashSupplierCalls);
		EXPECT_EQ(0u, context.TransactionRangeConsumerCalls);
		EXPECT_EQ(0u, context.BlockRangeConsumerCalls);

		const auto& actualShortHashRange = context.pTransactionApi->unconfirmedTransactionsRequests()[0];
		AssertEqualRange(shortHashRange, actualShortHashRange, "short hash");
	}

	// endregion

	// region unprocessed elements

	namespace {
		enum class ChainScoreRelation {
			RemoteBetter,
			Equal
		};

		auto CreateTestContextForUnprocessedElementTests(ChainScoreRelation relation = ChainScoreRelation::RemoteBetter) {
			auto remoteHashes = test::GenerateRandomHashes(10);
			auto localHashes = test::GenerateRandomHashesSubset(remoteHashes, 9);
			auto context = TestContext(
					ChainScore(10),
					ChainScoreRelation::RemoteBetter == relation ? ChainScore(11) : ChainScore(10),
					localHashes,
					remoteHashes,
					test::GenerateVerifiableBlockAtHeight(Default_Height));
			context.pChainApi->setNumBlocksPerBlocksFromRequest({ 2 });
			context.Config.MaxRollbackBlocks = 9;
			return context;
		}

		void AssertRequestHeight(const TestContext& context, size_t index, Height expectedHeight) {
			EXPECT_EQ(expectedHeight, context.pChainApi->blocksFromRequests()[index].first)
					<< "height for request " << (index + 1);
		}

		void AssertRequestHeights(const TestContext& context, const std::vector<Height>& expectedHeights) {
			ASSERT_EQ(expectedHeights.size(), context.pChainApi->blocksFromRequests().size());

			auto i = 0u;
			for (auto expectedHeight : expectedHeights)
				AssertRequestHeight(context, i++, expectedHeight);
		}
	}

	TEST(ChainSynchronizerTests, SuccessfulInteractionIfContainerIsNeitherFullNorDirty) {
		// Arrange: by default the container max size is large enough to pass many sync rounds without being full
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// Act:
		for (auto i = 0u; i < 10u; i++)
			interactionResults.push_back(synchronizer(context.Remote).get());

		// Assert:
		AssertSync(context, 10);

		auto i = 0u;
		ASSERT_EQ(10u, context.pChainApi->blocksFromRequests().size());
		for (auto result : interactionResults) {
			EXPECT_EQ(NodeInteractionResult::Success, result);
			AssertRequestHeight(context, i, Default_Height + Height(2 * i));
			++i;
		}
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfContainerIsFull) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(Block) / 3 - 1;
		auto synchronizer = CreateSynchronizer(context);

		// Act: second call is short circuited since the container is full
		auto result1 = synchronizer(context.Remote).get();
		auto result2 = synchronizer(context.Remote).get();

		// Assert:
		EXPECT_EQ(result1, NodeInteractionResult::Success);
		EXPECT_EQ(result2, NodeInteractionResult::Neutral);
		AssertSync(context, 1);

		AssertRequestHeights(context, { Default_Height });
	}

	TEST(ChainSynchronizerTests, SuccessfulInteractionIfPreviouslyFullContainerIsNoLongerFull) {
		// Arrange: the container's max size is set to 3 * MaxChainBytesPerSyncAttempt = 3 * sizeof(Block)
		//          that means the container is full after 2 syncs (2 blocks per sync)
		auto context = CreateTestContextForUnprocessedElementTests();
		context.Config.MaxChainBytesPerSyncAttempt = sizeof(Block);
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// - sucessful since container is not full
		interactionResults.push_back(synchronizer(context.Remote).get());
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - neutral since container is full
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - signal processing for first element has finished
		context.ProcessingComplete(1, CreateContinueResult());

		// Act: since an unprocessed element was removed, the container is no longer full
		interactionResults.push_back(synchronizer(context.Remote).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height + Height(4) });
	}

	TEST(ChainSynchronizerTests, NeutralInteractionIfContainerIsDirty) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;
		interactionResults.push_back(synchronizer(context.Remote).get());
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// Act: neutral because container is dirty
		interactionResults.push_back(synchronizer(context.Remote).get());
		interactionResults.push_back(synchronizer(context.Remote).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Neutral
		};

		AssertSync(context, 2);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
	}

	TEST(ChainSynchronizerTests, DirtyContainerIsCleanAfterAllElementsHaveBeenRemoved) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;
		interactionResults.push_back(synchronizer(context.Remote).get());
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - signal processing for first element has finished unsuccessfully making the container dirty
		context.ProcessingComplete(1, CreateAbortResult());

		// - neutral because container is dirty
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - signal processing for second element has finished. The container is empty and thus clean again
		context.ProcessingComplete(2, CreateAbortResult());

		// Act: successful because container is clean
		interactionResults.push_back(synchronizer(context.Remote).get());

		// Assert:
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral,
			NodeInteractionResult::Success
		};

		AssertSync(context, 3);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		// - last request should be Default_Height because container is empty again
		AssertRequestHeights(context, { Default_Height, Default_Height + Height(2), Default_Height });
	}

	TEST(ChainSynchronizerTests, PendingBlocksAreMarkedAsDirtyIfRootBlocksAreDirty) {
		// Arrange: non-deterministic because of dependency on delay
		test::RunNonDeterministicTest("closes socket test", [](auto i) {
			// Arrange:
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);
			std::vector<NodeInteractionResult> interactionResults;

			// Act: start an (immediate) request
			interactionResults.push_back(synchronizer(context.Remote).get());

			// - start a delayed request
			context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10 * i));
			auto syncFuture = synchronizer(context.Remote);

			// - signal processing for first element has finished unsuccessfully making the container dirty
			context.ProcessingComplete(1, CreateAbortResult());

			// - wait for the delayed request (it must still be pending in order for this test to give a deterministic result)
			if (syncFuture.is_ready())
				return false;

			interactionResults.push_back(syncFuture.get());

			// Assert:
			std::vector<NodeInteractionResult> expectedInteractionResults{
				NodeInteractionResult::Success,
				NodeInteractionResult::Neutral
			};

			// - only one range was forwarded to the consumer because the second was rejected
			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResults, interactionResults);

			AssertRequestHeights(context, { Default_Height, Default_Height + Height(2) });
			return true;
		});
	}

	TEST(ChainSynchronizerTests, MultiplePendingSyncsAreNotAllowedAtOnce) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// Act: start two delayed requests
		context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
		auto syncFuture1 = synchronizer(context.Remote);
		auto syncFuture2 = synchronizer(context.Remote);

		// - wait for the delayed requests
		interactionResults.push_back(syncFuture1.get());
		interactionResults.push_back(syncFuture2.get());

		// Assert: only one outstanding sync is allowed at a time
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Success,
			NodeInteractionResult::Neutral
		};

		// - only one range was forwarded to the consumer because the second was bypassed
		AssertSync(context, 1);
		EXPECT_EQ(expectedInteractionResults, interactionResults);

		// - only one request was made because the second was bypassed
		AssertRequestHeights(context, { Default_Height });
	}

	TEST(ChainSynchronizerTests, ThrowsIfCompletionHandlerIsCalledWithUnknownElementId) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests();
		auto synchronizer = CreateSynchronizer(context);
		auto resultFuture1 = synchronizer(context.Remote);

		// Act:
		// - signal processing has finished with unknown element id
		EXPECT_THROW(context.ProcessingComplete(123, CreateContinueResult()), catapult_invalid_argument);
	}

	// endregion

	// region recoverability

	namespace {
		void AssertSyncRecoverablityAfterChainApiException(
				MockChainApi::EntryPoint errorPoint,
				const std::vector<Height>& expectedRequestHeights) {
			// Arrange:
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);
			std::vector<NodeInteractionResult> interactionResults;

			// Act: set an exception and sync
			context.pChainApi->setError(errorPoint);
			interactionResults.push_back(synchronizer(context.Remote).get());

			// - clear the exception and sync
			context.pChainApi->setError(MockChainApi::EntryPoint::None);
			interactionResults.push_back(synchronizer(context.Remote).get());

			// Assert: the first sync failed but the second succeeded
			std::vector<NodeInteractionResult> expectedInteractionResults{
				NodeInteractionResult::Failure,
				NodeInteractionResult::Success
			};

			AssertSync(context, 1);
			EXPECT_EQ(expectedInteractionResults, interactionResults);
			AssertRequestHeights(context, expectedRequestHeights);
		}
	}

	TEST(ChainSynchronizerTests, CanRecoverAfterFailedCompareChainsOperation) {
		// Assert: only one blocks-from request was made because the chain comparison failed
		AssertSyncRecoverablityAfterChainApiException(
				MockChainApi::EntryPoint::Chain_Info,
				{ Default_Height });
	}

	TEST(ChainSynchronizerTests, CanRecoverAfterFailedBlocksFromOperation) {
		// Assert: two blocks-from requests were made beacuse the the chain comparison succeeded
		AssertSyncRecoverablityAfterChainApiException(
				MockChainApi::EntryPoint::Blocks_From,
				{ Default_Height, Default_Height });
	}

	TEST(ChainSynchronizerTests, CanRecoverAfterNeutralCompareChainsOperation) {
		// Arrange:
		auto context = CreateTestContextForUnprocessedElementTests(ChainScoreRelation::Equal);
		auto synchronizer = CreateSynchronizer(context);
		std::vector<NodeInteractionResult> interactionResults;

		// Act: set an exception and sync
		context.pTransactionApi->setError(MockTransactionApi::EntryPoint::Unconfirmed_Transactions);
		interactionResults.push_back(synchronizer(context.Remote).get());

		// - clear the exception and sync
		context.pTransactionApi->setError(MockTransactionApi::EntryPoint::None);
		interactionResults.push_back(synchronizer(context.Remote).get());

		// Assert: the first sync failed but the second succeeded
		std::vector<NodeInteractionResult> expectedInteractionResults{
			NodeInteractionResult::Failure,
			NodeInteractionResult::Neutral
		};

		EXPECT_EQ(2u, context.ShortHashSupplierCalls);
		EXPECT_EQ(1u, context.TransactionRangeConsumerCalls);
		EXPECT_EQ(0u, context.BlockRangeConsumerCalls);
		EXPECT_EQ(expectedInteractionResults, interactionResults);
		AssertRequestHeights(context, {}); // no blocks should have been requested
	}

	// endregion

	// region clean shutdown

	TEST(ChainSynchronizerTests, SynchronizerFutureCanCompleteAfterSynchronizerIsDestroyed) {
		// Arrange:
		thread::future<NodeInteractionResult> future;
		auto context = CreateDefaultTestContext(9, 10);
		{
			context.pChainApi->setDelay(utils::TimeSpan::FromMilliseconds(10));
			auto synchronizer = CreateSynchronizer(context);

			// Act: start synchronizing
			future = synchronizer(context.Remote);

			// - destroy the synchronizer before completion
			CATAPULT_LOG(debug) << "destroying synchronizer";
		}

		// Sanity: future has not yet completed
		EXPECT_FALSE(future.is_ready());

		// Assert: future can complete
		future.get();
	}

	TEST(ChainSynchronizerTests, BlockRangeConsumerCanCompleteAfterSynchronizerIsDestroyed) {
		// Arrange:
		disruptor::ProcessingCompleteFunc processingComplete;
		{
			auto context = CreateTestContextForUnprocessedElementTests();
			auto synchronizer = CreateSynchronizer(context);

			// - add an element to unprocessed elements and capture the completion function
			synchronizer(context.Remote).get();
			processingComplete = context.ProcessingComplete;

			// Act: destroy the synchronizer before completion
			CATAPULT_LOG(debug) << "destroying synchronizer";
		}

		// Assert: the completion function can still be called even though the synchronizer has been destroyed
		processingComplete(1, CreateAbortResult());
	}

	// endregion
}}
