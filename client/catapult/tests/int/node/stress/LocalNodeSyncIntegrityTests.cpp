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

#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/int/node/stress/test/BlockChainBuilder.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/int/node/test/PeerLocalNodeTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeSyncIntegrityTests

	namespace {
		using TestContext = test::PeerLocalNodeTestContext;

		using StateHashCalculator = test::StateHashCalculator;
		using Accounts = test::Accounts;
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;

		// region AssertXemBalances/ AssertNamespaceCount

		struct ExpectedBalance {
			size_t AccountId;
			Amount Balance;
		};

		void AssertXemBalances(
				const Accounts& accounts,
				const cache::CatapultCache& cache,
				const std::vector<ExpectedBalance>& expectedBalances) {
			auto cacheView = cache.createView();
			const auto& accountStateCache = cacheView.sub<cache::AccountStateCache>();

			for (const auto& expectedBalance : expectedBalances) {
				const auto& address = accounts.getAddress(expectedBalance.AccountId);

				std::ostringstream out;
				out << "account id " << expectedBalance.AccountId << " (" << model::AddressToString(address) << ")";
				auto message = out.str();

				// Assert:
				auto accountStateIter = accountStateCache.find(address);
				EXPECT_TRUE(!!accountStateIter.tryGet()) << message;
				if (!accountStateIter.tryGet())
					continue;

				const auto& accountState = accountStateIter.get();
				EXPECT_EQ(1u, accountState.Balances.size()) << message;
				EXPECT_EQ(expectedBalance.Balance, accountState.Balances.get(Xem_Id)) << message;
			}
		}

		void AssertNamespaceCount(const BootedLocalNode& localNode, size_t numExpectedNamespaces) {
			// Assert:
			auto numNamespaces = test::GetCounterValue(localNode.counters(), "NS C");
			EXPECT_EQ(numExpectedNamespaces, numNamespaces);
		}

		// endregion

		// region test contexts

		class StateHashDisabledTestContext : public TestContext {
			public:
				StateHashDisabledTestContext() : TestContext(test::NodeFlag::Regular)
				{}

			public:
				StateHashCalculator createStateHashCalculator() const {
					return StateHashCalculator();
				}
		};

		class StateHashEnabledTestContext : public TestContext {
			public:
				StateHashEnabledTestContext()
						: TestContext(test::NodeFlag::Verify_State)
						, m_stateHashCalculationDir("../temp.dir_statehash") // isolated directory used for state hash calculation
				{}

			public:
				StateHashCalculator createStateHashCalculator() const {
					{
						test::TempDirectoryGuard forceCleanResourcesDir(m_stateHashCalculationDir.name());
					}

					return StateHashCalculator(prepareFreshDataDirectory(m_stateHashCalculationDir.name()));
				}

			private:
				test::TempDirectoryGuard m_stateHashCalculationDir;
		};

		// endregion

		// region traits

		struct SingleBlockTraits {
			static auto GetBlocks(BlockChainBuilder& builder) {
				return Blocks{ utils::UniqueToShared(builder.asSingleBlock()) };
			}
		};

		struct MultiBlockTraits {
			static auto GetBlocks(BlockChainBuilder& builder) {
				return builder.asBlockChain();
			}
		};

#define SINGLE_MULTI_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_SingleBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleBlockTraits>(); } \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_BlockChain) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultiBlockTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion

		// region utils

		Hash256 GetStateHash(const TestContext& context) {
			return context.localNode().cache().createView().calculateStateHash().StateHash;
		}

		Hash256 GetNamespaceStateHash(const TestContext& context) {
			auto subCacheMerkleRoots = context.localNode().cache().createView().calculateStateHash().SubCacheMerkleRoots;
			return subCacheMerkleRoots.empty() ? Hash256() : subCacheMerkleRoots[1]; // namespace state hash is second
		}

		void WaitForHeightAndElements(
				const TestContext& context,
				Height height,
				uint32_t numExpectedBlockElements,
				uint32_t numTerminalReaders) {
			// Act: wait for the chain height to change and for all height readers to disconnect
			context.waitForHeight(height);
			auto chainHeight = context.height();
			WAIT_FOR_VALUE_EXPR(numTerminalReaders, context.stats().NumActiveReaders);

			// Assert: the chain height is correct
			EXPECT_EQ(height, chainHeight);

			// - a single block element was added
			auto stats = context.stats();
			EXPECT_EQ(numExpectedBlockElements, stats.NumAddedBlockElements);
			EXPECT_EQ(0u, stats.NumAddedTransactionElements);

			// - the connection is still active
			EXPECT_EQ(numTerminalReaders, stats.NumActiveReaders);
		}

		void ResignBlock(model::Block& block) {
			for (const auto* pPrivateKeyString : test::Mijin_Test_Private_Keys) {
				auto keyPair = crypto::KeyPair::FromString(pPrivateKeyString);
				if (keyPair.publicKey() == block.Signer) {
					test::SignBlock(keyPair, block);
					return;
				}
			}

			CATAPULT_THROW_RUNTIME_ERROR("unable to find block signer among mijin test private keys");
		}

		void AssertAllZero(const std::vector<Hash256>& hashes, size_t numExpected) {
			// Sanity:
			EXPECT_EQ(numExpected, hashes.size());

			// Assert:
			auto i = 0u;
			for (const auto& hash : hashes)
				EXPECT_EQ(Hash256(), hash) << "hash at " << i;
		}

		void AssertAllNonZero(const std::vector<Hash256>& hashes, size_t numExpected) {
			// Sanity:
			EXPECT_EQ(numExpected, hashes.size());

			// Assert:
			auto i = 0u;
			for (const auto& hash : hashes)
				EXPECT_NE(Hash256(), hash) << "hash at " << i;
		}

		void AssertUnique(const std::vector<Hash256>& hashes) {
			EXPECT_EQ(hashes.size(), utils::HashSet(hashes.cbegin(), hashes.cend()).size());
		}

		// endregion
	}

	// region transfer application (success)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunApplyTest(TTestContext& context) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			test::WaitForBoot(context);
			stateHashes.push_back(GetStateHash(context));

			// Sanity:
			EXPECT_EQ(Height(1), context.height());

			// - prepare transfers (all transfers are dependent on previous transfer)
			Accounts accounts(6);
			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addTransfer(0, 1, Amount(1'000'000));
			builder.addTransfer(1, 2, Amount(900'000));
			builder.addTransfer(2, 3, Amount(700'000));
			builder.addTransfer(3, 4, Amount(400'000));
			builder.addTransfer(4, 5, Amount(50'000));
			auto blocks = TTraits::GetBlocks(builder);

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(1 + blocks.size()), 1, 1);
			stateHashes.push_back(GetStateHash(context));

			// Assert: the cache has expected balances
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(100'000) },
				{ 2, Amount(200'000) },
				{ 3, Amount(300'000) },
				{ 4, Amount(350'000) },
				{ 5, Amount(50'000) }
			});

			return stateHashes;
		}
	}

	SINGLE_MULTI_BASED_TEST(CanApplyTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (success)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::pair<BlockChainBuilder, Blocks> PrepareFiveChainedTransfers(
				TTestContext& context,
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator) {
			// Arrange:
			test::WaitForBoot(context);

			// Sanity:
			EXPECT_EQ(Height(1), context.height());

			// - prepare transfers (all transfers are dependent on previous transfer)
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addTransfer(0, 1, Amount(1'000'000));
			builder.addTransfer(1, 2, Amount(900'000));
			builder.addTransfer(2, 3, Amount(700'000));
			builder.addTransfer(3, 4, Amount(400'000));
			builder.addTransfer(4, 5, Amount(50'000));
			auto blocks = TTraits::GetBlocks(builder);

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(1 + blocks.size()), 1, 1);
			return std::make_pair(builder, blocks);
		}

		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRollbackTest(TTestContext& context) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			Accounts accounts(6);
			{
				// - always use SingleBlockTraits because a push can rollback at most one block
				auto stateHashCalculator = context.createStateHashCalculator();
				PrepareFiveChainedTransfers<SingleBlockTraits>(context, accounts, stateHashCalculator);
				stateHashes.push_back(GetStateHash(context));
			}

			// - prepare transfers (all are from nemesis)
			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder1(accounts, stateHashCalculator);
			builder1.setBlockTimeInterval(Timestamp(58'000)); // better block time will yield better chain
			builder1.addTransfer(0, 1, Amount(1'000'000));
			builder1.addTransfer(0, 2, Amount(900'000));
			builder1.addTransfer(0, 3, Amount(700'000));
			builder1.addTransfer(0, 4, Amount(400'000));
			builder1.addTransfer(0, 5, Amount(50'000));
			auto blocks = TTraits::GetBlocks(builder1);

			// - prepare a transfer that can only attach to rollback case
			auto builder2 = builder1.createChainedBuilder();
			builder2.addTransfer(2, 4, Amount(350'000));
			auto pTailBlock = utils::UniqueToShared(builder2.asSingleBlock());

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(1 + blocks.size() + 1), 3, 2);
			stateHashes.push_back(GetStateHash(context));

			// Assert: the cache has expected balances
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(1'000'000) },
				{ 2, Amount(550'000) },
				{ 3, Amount(700'000) },
				{ 4, Amount(750'000) },
				{ 5, Amount(50'000) }
			});

			return stateHashes;
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRollbackTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application (validation failure)

	namespace {
		void SeedStateHashCalculator(StateHashCalculator& stateHashCalculator, const Blocks& blocks) {
			// can load nemesis from memory because it is only used for execution, so state hash can be wrong
			mocks::MockMemoryBlockStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));
			stateHashCalculator.execute(pNemesisBlockElement->Block);

			for (const auto& pBlock : blocks)
				stateHashCalculator.execute(*pBlock);
		}

		template<typename TTraits, typename TTestContext, typename TGenerateInvalidBlocks>
		std::vector<Hash256> RunRejectInvalidApplyTest(TTestContext& context, TGenerateInvalidBlocks generateInvalidBlocks) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			Accounts accounts(6);
			std::unique_ptr<BlockChainBuilder> pBuilder1;
			Blocks seedBlocks;
			{
				// - seed the chain with initial blocks
				auto stateHashCalculator = context.createStateHashCalculator();
				auto builderBlocksPair = PrepareFiveChainedTransfers<TTraits>(context, accounts, stateHashCalculator);
				pBuilder1 = std::make_unique<BlockChainBuilder>(builderBlocksPair.first);
				seedBlocks = builderBlocksPair.second;
				stateHashes.push_back(GetStateHash(context));
			}

			Blocks invalidBlocks;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare invalid blocks
				auto builder2 = pBuilder1->createChainedBuilder(stateHashCalculator);
				invalidBlocks = generateInvalidBlocks(builder2);
			}

			std::shared_ptr<model::Block> pTailBlock;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare a transfer that can only attach to initial blocks
				auto builder3 = pBuilder1->createChainedBuilder(stateHashCalculator);
				builder3.addTransfer(1, 2, Amount(91'000));
				pTailBlock = utils::UniqueToShared(builder3.asSingleBlock());
			}

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, invalidBlocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(1 + seedBlocks.size() + 1), 3, 2);
			stateHashes.push_back(GetStateHash(context));

			// Assert: the cache has expected balances
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(9'000) },
				{ 2, Amount(291'000) },
				{ 3, Amount(300'000) },
				{ 4, Amount(350'000) },
				{ 5, Amount(50'000) }
			});

			return stateHashes;
		}

		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidValidationApplyTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidApplyTest<TTraits>(context, [](auto& builder) {
				// Arrange: prepare three transfers, where second is invalid
				builder.addTransfer(1, 5, Amount(90'000));
				builder.addTransfer(2, 5, Amount(200'001));
				builder.addTransfer(3, 5, Amount(80'000));
				return TTraits::GetBlocks(builder);
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationApplyTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application (state hash failure)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidStateHashApplyTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidApplyTest<TTraits>(context, [](auto& builder) {
				// Arrange: prepare three valid transfers
				builder.addTransfer(1, 5, Amount(90'000));
				builder.addTransfer(2, 5, Amount(80'000));
				builder.addTransfer(3, 5, Amount(80'000));
				auto blocks = TTraits::GetBlocks(builder);

				// - corrupt state hash of last block
				test::FillWithRandomData(blocks.back()->StateHash);
				ResignBlock(*blocks.back());
				return blocks;
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashApplyTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (validation failure)

	namespace {
		template<typename TTraits, typename TTestContext, typename TGenerateInvalidBlocks>
		std::vector<Hash256> RunRejectInvalidRollbackTest(TTestContext& context, TGenerateInvalidBlocks generateInvalidBlocks) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			Accounts accounts(6);
			std::unique_ptr<BlockChainBuilder> pBuilder1;
			Blocks seedBlocks;
			{
				// - seed the chain with initial blocks
				// - always use SingleBlockTraits because a push can rollback at most one block
				auto stateHashCalculator = context.createStateHashCalculator();
				auto builderBlocksPair = PrepareFiveChainedTransfers<SingleBlockTraits>(context, accounts, stateHashCalculator);
				pBuilder1 = std::make_unique<BlockChainBuilder>(builderBlocksPair.first);
				seedBlocks = builderBlocksPair.second;
				stateHashes.push_back(GetStateHash(context));
			}

			Blocks invalidBlocks;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				BlockChainBuilder builder2(accounts, stateHashCalculator);

				// - prepare invalid blocks
				builder2.setBlockTimeInterval(Timestamp(58'000)); // better block time will yield better chain
				invalidBlocks = generateInvalidBlocks(builder2);
			}

			std::shared_ptr<model::Block> pTailBlock;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare a transfer that can only attach to initial blocks
				auto builder3 = pBuilder1->createChainedBuilder(stateHashCalculator);
				builder3.addTransfer(1, 2, Amount(91'000));
				pTailBlock = utils::UniqueToShared(builder3.asSingleBlock());
			}

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, invalidBlocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(1 + seedBlocks.size() + 1), 3, 2);
			stateHashes.push_back(GetStateHash(context));

			// Assert: the cache has expected balances
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(9'000) },
				{ 2, Amount(291'000) },
				{ 3, Amount(300'000) },
				{ 4, Amount(350'000) },
				{ 5, Amount(50'000) }
			});

			return stateHashes;
		}

		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidValidationRollbackTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidRollbackTest<TTraits>(context, [](auto& builder) {
				// Arrange: prepare five transfers, where third is invalid
				builder.addTransfer(0, 1, Amount(10'000));
				builder.addTransfer(0, 2, Amount(900'000));
				builder.addTransfer(2, 3, Amount(900'001));
				builder.addTransfer(0, 4, Amount(400'000));
				builder.addTransfer(0, 5, Amount(50'000));
				return TTraits::GetBlocks(builder);
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationRollbackTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (state hash failure)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidStateHashRollbackTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidRollbackTest<TTraits>(context, [](auto& builder) {
				// Arrange: prepare five valid transfers
				builder.addTransfer(0, 1, Amount(10'000));
				builder.addTransfer(0, 2, Amount(900'000));
				builder.addTransfer(0, 3, Amount(900'001));
				builder.addTransfer(0, 4, Amount(400'000));
				builder.addTransfer(0, 5, Amount(50'000));
				auto blocks = TTraits::GetBlocks(builder);

				// - corrupt state hash of last block
				test::FillWithRandomData(blocks.back()->StateHash);
				ResignBlock(*blocks.back());
				return blocks;
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashRollbackTransactions) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashes, 2);
		AssertUnique(stateHashes);
	}

	// endregion

	namespace {
		// region namespace specific utils

		using NamespaceStateHashes = std::vector<std::pair<Hash256, Hash256>>;

		template<typename T>
		std::pair<std::vector<T>, std::vector<T>> Unzip(const std::vector<std::pair<T, T>>& pairs) {
			std::pair<std::vector<T>, std::vector<T>> result;
			for (const auto& pair : pairs) {
				result.first.push_back(pair.first);
				result.second.push_back(pair.second);
			}

			return result;
		}

		template<typename TTestContext>
		std::pair<BlockChainBuilder, std::shared_ptr<model::Block>> PrepareTwoRootNamespaces(
				TTestContext& context,
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				NamespaceStateHashes& stateHashes) {
			// Arrange:
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity:
			EXPECT_EQ(Height(1), context.height());
			AssertNamespaceCount(context.localNode(), 1);

			// - prepare namespace registrations
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addNamespace(0, "foo", BlockDuration(12));
			builder.addNamespace(0, "bar", BlockDuration(12));
			auto pNamespaceBlock = utils::UniqueToShared(builder.asSingleBlock());

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pNamespaceBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity: the cache has expected namespaces
			AssertNamespaceCount(context.localNode(), 3);

			return std::make_pair(builder, std::move(pNamespaceBlock));
		}

		struct XemTransferBlocksResult {
			BlockChainBuilder Builder;
			Blocks AllBlocks;
			uint32_t NumAliveChains;
		};

		template<typename TTestContext>
		XemTransferBlocksResult PushXemTransferBlocks(
				TTestContext& context,
				test::ExternalSourceConnection& connection,
				BlockChainBuilder& builder,
				size_t numTotalBlocks) {
			Blocks allBlocks;
			auto numAliveChains = 0u;
			auto numRemainingBlocks = numTotalBlocks;
			for (;;) {
				auto numBlocks = std::min<size_t>(50, numRemainingBlocks);
				for (auto i = 0u; i < numBlocks; ++i)
					builder.addTransfer(0, 1, Amount(1));

				auto blocks = builder.asBlockChain();
				auto pIo = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

				numRemainingBlocks -= numBlocks;
				++numAliveChains;
				allBlocks.insert(allBlocks.end(), blocks.cbegin(), blocks.cend());

				WaitForHeightAndElements(context, Height(2 + numTotalBlocks - numRemainingBlocks), 1 + numAliveChains, 1);
				if (0 == numRemainingBlocks)
					break;

				builder = builder.createChainedBuilder();
			}

			return XemTransferBlocksResult{ builder, allBlocks, numAliveChains };
		}

		// endregion
	}

	// region namespace (register)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunRegisterNamespaceTest(TTestContext& context) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity:
			EXPECT_EQ(Height(1), context.height());
			AssertNamespaceCount(context.localNode(), 1);

			// - prepare namespace registrations
			Accounts accounts(1);
			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addNamespace(0, "foo", BlockDuration(10));
			builder.addNamespace(0, "bar", BlockDuration(10));
			auto pNamespaceBlock = utils::UniqueToShared(builder.asSingleBlock());

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pNamespaceBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has expected namespaces
			AssertNamespaceCount(context.localNode(), 3);

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespace) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunRegisterNamespaceTest(context));

		// Assert: all state hashes are zero
		AssertAllZero(stateHashesPair.first, 2);

		// - all namespace cache merkle roots are zero
		AssertAllZero(stateHashesPair.second, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespaceWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunRegisterNamespaceTest(context));

		// Assert: all state hashes are nonzero
		AssertAllNonZero(stateHashesPair.first, 2);
		AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		AssertAllNonZero(stateHashesPair.second, 2);
		AssertUnique(stateHashesPair.second);
	}

	// endregion

	// region namespace (expire)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunNamespaceStateChangeTest(TTestContext& context, size_t numAliveBlocks, size_t numExpectedNamespaces) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			Accounts accounts(2);
			auto stateHashCalculator = context.createStateHashCalculator();
			auto builderBlockPair = PrepareTwoRootNamespaces(context, accounts, stateHashCalculator, stateHashes);
			auto& builder = builderBlockPair.first;

			// - add the specified number of blocks up to a state change
			test::ExternalSourceConnection connection;
			auto builder2 = builder.createChainedBuilder();
			auto xemTransferBlocksResult = PushXemTransferBlocks(context, connection, builder2, numAliveBlocks);
			auto numAliveChains = xemTransferBlocksResult.NumAliveChains;
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity: all namespaces are still present
			AssertNamespaceCount(context.localNode(), 3);

			// - prepare a block that triggers a state change
			auto builder3 = xemTransferBlocksResult.Builder.createChainedBuilder();
			builder3.addTransfer(0, 1, Amount(1));
			auto pTailBlock = utils::UniqueToShared(builder3.asSingleBlock());

			// Act:
			auto pIo1 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 1), 1 + numAliveChains + 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has the expected balances and namespaces
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 1) }
			});
			AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

			return stateHashes;
		}

		template<typename TTestContext>
		NamespaceStateHashes RunExpireNamespaceTest(TTestContext& context) {
			// Arrange: namespace expires after the sum of the following
			//          (1) namespace duration => 12
			auto numAliveBlocks = 12u - 1;

			// Act:
			return RunNamespaceStateChangeTest(context, numAliveBlocks, 3);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireNamespace) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireNamespaceTest(context));

		// Assert: all state hashes are zero
		AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireNamespaceWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		AssertAllNonZero(stateHashesPair.first, 4);
		AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[1], stateHashesPair.second[2]); // hash includes new namespaces
		EXPECT_EQ(stateHashesPair.second[2], stateHashesPair.second[3]); // hash includes new namespaces (expired but in grace period)
		EXPECT_NE(stateHashesPair.second[0], stateHashesPair.second[3]);
	}

	// endregion

	// region namespace (prune)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunPruneNamespaceTest(TTestContext& context) {
			// Arrange: namespace is pruned after the sum of the following
			//          (1) namespace duration ==> 12
			//          (2) grace period ========> 1hr of blocks with 20s target time
			//          (3) max rollback blocks => 10
			auto numAliveBlocks = static_cast<uint32_t>(12 + (utils::TimeSpan::FromHours(1).seconds() / 20) + 10 - 1);

			// Act:
			return RunNamespaceStateChangeTest(context, numAliveBlocks, 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespace) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneNamespaceTest(context));

		// Assert: all state hashes are zero
		AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespaceWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		AssertAllNonZero(stateHashesPair.first, 4);
		AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]); // hash does not include new namespaces (expired)
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[3]); // hash does not include new namespaces (pruned)
	}

	// endregion

	// region namespace (expire + rollback)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunNamespaceStateChangeRollbackTest(
				TTestContext& context,
				size_t numAliveBlocks,
				size_t numExpectedNamespaces) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			Accounts accounts(2);
			std::unique_ptr<BlockChainBuilder> pBuilder;
			std::vector<std::shared_ptr<model::Block>> allBlocks;
			uint32_t numAliveChains;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				auto builderBlockPair = PrepareTwoRootNamespaces(context, accounts, stateHashCalculator, stateHashes);

				// - add the specified number of blocks up to a state change
				test::ExternalSourceConnection connection;
				auto builder2 = builderBlockPair.first.createChainedBuilder();
				auto xemTransferBlocksResult = PushXemTransferBlocks(context, connection, builder2, numAliveBlocks);
				numAliveChains = xemTransferBlocksResult.NumAliveChains;
				stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

				// Sanity: all namespaces are still present
				AssertNamespaceCount(context.localNode(), 3);

				allBlocks.push_back(builderBlockPair.second);
				allBlocks.insert(allBlocks.end(), xemTransferBlocksResult.AllBlocks.cbegin(), xemTransferBlocksResult.AllBlocks.cend());
				pBuilder = std::make_unique<BlockChainBuilder>(builder2);
			}

			// - prepare a block that triggers a state change
			std::shared_ptr<model::Block> pExpiryBlock1;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				SeedStateHashCalculator(stateHashCalculator, allBlocks);

				auto builder3 = pBuilder->createChainedBuilder(stateHashCalculator);
				builder3.addTransfer(0, 1, Amount(1));
				pExpiryBlock1 = utils::UniqueToShared(builder3.asSingleBlock());
			}

			// - prepare two blocks that triggers a state change
			Blocks expiryBlocks2;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				SeedStateHashCalculator(stateHashCalculator, allBlocks);

				auto builder3 = pBuilder->createChainedBuilder(stateHashCalculator);
				builder3.setBlockTimeInterval(Timestamp(58'000)); // better block time will yield better chain
				builder3.addTransfer(0, 1, Amount(1));
				builder3.addTransfer(0, 1, Amount(1));
				expiryBlocks2 = builder3.asBlockChain();
			}

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo1 = test::PushEntity(connection, ionet::PacketType::Push_Block, pExpiryBlock1);
			auto pIo2 = test::PushEntities(connection, ionet::PacketType::Push_Block, expiryBlocks2);

			// - wait for the chain height to change and for all height readers to disconnect
			WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 2), 1 + numAliveChains + 2, 2);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has the expected balances and namespaces
			AssertXemBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 2) }
			});
			AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

			return stateHashes;
		}

		template<typename TTestContext>
		NamespaceStateHashes RunExpireAndRollbackNamespaceTest(TTestContext& context) {
			// Arrange: namespace expires after the sum of the following
			//          (1) namespace duration => 12
			auto numAliveBlocks = 12u - 1;

			// Act:
			return RunNamespaceStateChangeRollbackTest(context, numAliveBlocks, 3);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireAndRollbackNamespace) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert: all state hashes are zero
		AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		AssertAllNonZero(stateHashesPair.first, 4);
		AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[1], stateHashesPair.second[2]); // hash includes new namespaces
		EXPECT_EQ(stateHashesPair.second[2], stateHashesPair.second[3]); // hash includes new namespaces (expired but in grace period)
		EXPECT_NE(stateHashesPair.second[0], stateHashesPair.second[3]);
	}

	// endregion

	// region namespace (prune + rollback)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunPruneAndRollbackNamespaceTest(TTestContext& context) {
			// Arrange: namespace is pruned after the sum of the following
			//          (1) namespace duration ==> 12
			//          (2) grace period ========> 1hr of blocks with 20s target time
			//          (3) max rollback blocks => 10
			auto numAliveBlocks = static_cast<uint32_t>(12 + (utils::TimeSpan::FromHours(1).seconds() / 20) + 10 - 1);

			// Act:
			return RunNamespaceStateChangeRollbackTest(context, numAliveBlocks, 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneAndRollbackNamespace) {
		// Arrange:
		StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert: all state hashes are zero
		AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		AssertAllNonZero(stateHashesPair.first, 4);
		AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]); // hash does not include new namespaces (expired)
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[3]); // hash does not include new namespaces (pruned)
	}

	// endregion
}}
