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

#include "tests/int/node/stress/test/ExpiryTestUtils.h"
#include "tests/int/node/stress/test/LocalNodeSyncIntegrityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeSyncNamespaceIntegrityTests

	namespace {
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;

		// region utils

		using NamespaceStateHashes = std::vector<std::pair<Hash256, Hash256>>;

		Hash256 GetComponentStateHash(const test::PeerLocalNodeTestContext& context) {
			auto subCacheMerkleRoots = context.localNode().cache().createView().calculateStateHash().SubCacheMerkleRoots;
			return subCacheMerkleRoots.empty() ? Hash256() : subCacheMerkleRoots[1]; // { AccountState, *Namespace*, Mosaic }
		}

		template<typename TTestContext>
		void AssertBooted(const TTestContext& context) {
			// Assert:
			EXPECT_EQ(Height(1), context.height());
			test::AssertNamespaceCount(context.localNode(), 1);
		}

		template<typename TTestContext>
		std::pair<BlockChainBuilder, std::shared_ptr<model::Block>> PrepareTwoRootNamespaces(
				TTestContext& context,
				const test::Accounts& accounts,
				test::StateHashCalculator& stateHashCalculator,
				NamespaceStateHashes& stateHashes) {
			// Arrange:
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Sanity:
			AssertBooted(context);

			// - prepare namespace registrations
			test::TransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addNamespace(0, "foo", BlockDuration(12));
			transactionsBuilder.addNamespace(0, "bar", BlockDuration(12));

			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto pNamespaceBlock = utils::UniqueToShared(builder.asSingleBlock(transactionsBuilder));

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pNamespaceBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Sanity: the cache has expected namespaces
			test::AssertNamespaceCount(context.localNode(), 3);

			return std::make_pair(builder, std::move(pNamespaceBlock));
		}

		// endregion

		// region TestFacade

		template<typename TTestContext>
		class TestFacade {
		public:
			explicit TestFacade(TTestContext& context)
					: m_context(context)
					, m_accounts(2)
			{}

		public:
			const auto& accounts() const {
				return m_accounts;
			}

			const auto& stateHashes() const {
				return m_stateHashes;
			}

			auto numAliveChains() const {
				return m_numAliveChains;
			}

		public:
			void pushNamespacesAndTransferBlocks(size_t numBlocks) {
				// Arrange: push a namespace registration block
				auto stateHashCalculator = m_context.createStateHashCalculator();
				auto builderBlockPair = PrepareTwoRootNamespaces(m_context, m_accounts, stateHashCalculator, m_stateHashes);

				// - add the specified number of blocks
				test::ExternalSourceConnection connection(m_context.publicKey());
				auto builder2 = builderBlockPair.first.createChainedBuilder();
				auto transferBlocksResult = PushTransferBlocks(m_context, connection, m_accounts, builder2, numBlocks);
				m_numAliveChains = transferBlocksResult.NumAliveChains;
				m_stateHashes.emplace_back(GetStateHash(m_context), GetComponentStateHash(m_context));

				// Sanity: all namespaces are still present
				test::AssertNamespaceCount(m_context.localNode(), 3);

				m_allBlocks.emplace_back(builderBlockPair.second);
				m_allBlocks.insert(m_allBlocks.end(), transferBlocksResult.AllBlocks.cbegin(), transferBlocksResult.AllBlocks.cend());
				m_pActiveBuilder = std::make_unique<BlockChainBuilder>(builder2);
			}

			Blocks createTailBlocks(utils::TimeSpan blockInterval, const consumer<test::TransactionsBuilder&>& addToBuilder) {
				auto stateHashCalculator = m_context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, m_allBlocks);

				test::TransactionsBuilder transactionsBuilder(m_accounts);
				addToBuilder(transactionsBuilder);

				auto builder = m_pActiveBuilder->createChainedBuilder(stateHashCalculator);
				builder.setBlockTimeInterval(blockInterval);
				return builder.asBlockChain(transactionsBuilder);
			}

		private:
			TTestContext& m_context;

			test::Accounts m_accounts;
			NamespaceStateHashes m_stateHashes;
			std::unique_ptr<BlockChainBuilder> m_pActiveBuilder;
			std::vector<std::shared_ptr<model::Block>> m_allBlocks;
			uint32_t m_numAliveChains;
		};

		// endregion
	}

	// region namespace (register)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunRegisterNamespaceTest(TTestContext& context) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			test::Accounts accounts(1);
			auto stateHashCalculator = context.createStateHashCalculator();

			// Act + Assert:
			PrepareTwoRootNamespaces(context, accounts, stateHashCalculator, stateHashes);

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRegisterNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRegisterNamespaceTest(context));

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 2);
		test::AssertUnique(stateHashesPair.second);
	}

	// endregion

	// region namespace (expire)

	namespace {
		// namespace expires after the sum of the following
		//   (1) namespace duration ==> 12
		constexpr auto Blocks_Before_Namespace_Expire = static_cast<uint32_t>(12);

		template<typename TTestContext>
		NamespaceStateHashes RunNamespaceStateChangeTest(TTestContext& context, size_t numAliveBlocks, size_t numExpectedNamespaces) {
			// Arrange: create namespace registration block followed by specified number of empty blocks
			TestFacade<TTestContext> facade(context);
			facade.pushNamespacesAndTransferBlocks(numAliveBlocks);

			// - prepare a block that triggers a state change
			auto nextBlocks = facade.createTailBlocks(utils::TimeSpan::FromSeconds(60), [](auto& transactionsBuilder) {
				transactionsBuilder.addTransfer(0, 1, Amount(1));
			});

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, nextBlocks);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 1), 1 + facade.numAliveChains() + 1, 1);

			// Assert: the cache has the expected balances and namespaces
			test::AssertCurrencyBalances(facade.accounts(), context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 1) }
			});
			test::AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

			auto stateHashes = facade.stateHashes();
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));
			return stateHashes;
		}

		template<typename TTestContext>
		NamespaceStateHashes RunExpireNamespaceTest(TTestContext& context) {
			// Act:
			return RunNamespaceStateChangeTest(context, Blocks_Before_Namespace_Expire - 1, 3);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunExpireNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunExpireNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[1], stateHashesPair.second[2]); // hash includes new namespaces
		EXPECT_EQ(stateHashesPair.second[2], stateHashesPair.second[3]); // hash includes new namespaces (expired but in grace period)
		EXPECT_NE(stateHashesPair.second[0], stateHashesPair.second[3]);
	}

	// endregion

	// region namespace (prune)

	namespace {
		// namespace is pruned after the sum of the following
		//   (1) namespace duration ==> 12
		//   (2) grace period ========> 1hr of blocks with 20s target time
		//   (3) max rollback blocks => 10
		constexpr auto Blocks_Before_Namespace_Prune = static_cast<uint32_t>(12 + (utils::TimeSpan::FromHours(1).seconds() / 20) + 10);

		template<typename TTestContext>
		NamespaceStateHashes RunPruneNamespaceTest(TTestContext& context) {
			// Act:
			return RunNamespaceStateChangeTest(context, Blocks_Before_Namespace_Prune, 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunPruneNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunPruneNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 4);
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
			// Arrange: create namespace registration block followed by specified number of empty blocks
			TestFacade<TTestContext> facade(context);
			facade.pushNamespacesAndTransferBlocks(numAliveBlocks);

			// - prepare two sets of blocks one of which will cause state change (better block time will yield better chain)
			auto worseBlocks = facade.createTailBlocks(utils::TimeSpan::FromSeconds(60), [](auto& transactionsBuilder) {
				transactionsBuilder.addTransfer(0, 1, Amount(1));
			});
			auto betterBlocks = facade.createTailBlocks(utils::TimeSpan::FromSeconds(58), [](auto& transactionsBuilder) {
				transactionsBuilder.addTransfer(0, 1, Amount(1));
				transactionsBuilder.addTransfer(0, 1, Amount(1));
			});

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, worseBlocks);
			auto pIo2 = test::PushEntities(connection, ionet::PacketType::Push_Block, betterBlocks);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 2), 1 + facade.numAliveChains() + 2, 2);

			// Assert: the cache has the expected balances and namespaces
			test::AssertCurrencyBalances(facade.accounts(), context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 2) }
			});
			test::AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

			auto stateHashes = facade.stateHashes();
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));
			return stateHashes;
		}

		template<typename TTestContext>
		NamespaceStateHashes RunExpireAndRollbackNamespaceTest(TTestContext& context) {
			// Act:
			return RunNamespaceStateChangeRollbackTest(context, Blocks_Before_Namespace_Expire - 1, 3);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireAndRollbackNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[1], stateHashesPair.second[2]); // hash includes new namespaces
		EXPECT_EQ(stateHashesPair.second[2], stateHashesPair.second[3]); // hash includes new namespaces (expired but in grace period)
		EXPECT_NE(stateHashesPair.second[0], stateHashesPair.second[3]);
	}

	// endregion

	// region namespace (prune + rollback)

	namespace {
		template<typename TTestContext>
		NamespaceStateHashes RunPruneAndRollbackNamespaceTest(TTestContext& context) {
			// Act:
			return RunNamespaceStateChangeRollbackTest(context, Blocks_Before_Namespace_Prune - 1, 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneAndRollbackNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]); // hash does not include new namespaces (expired)
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[3]); // hash does not include new namespaces (pruned)
	}

	// endregion

	// region namespace (register + deactivate) [single chain part]

	namespace {
		// namespace is deactivated after the sum of the following
		//   (1) namespace duration ==> 12
		//   (2) grace period ========> 1hr of blocks with 20s target time
		constexpr auto Blocks_Before_Namespace_Deactivate = static_cast<uint32_t>(12 + (utils::TimeSpan::FromHours(1).seconds() / 20));

		template<typename TTestContext>
		NamespaceStateHashes RunRegisterAndDeactivateNamespaceTest(TTestContext& context, size_t numAliveBlocks) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			test::Accounts accounts(2);
			auto stateHashCalculator = context.createStateHashCalculator();

			// - wait for boot
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Sanity:
			AssertBooted(context);

			// - prepare namespace registrations
			test::TransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addNamespace(0, "foo", BlockDuration(12));
			transactionsBuilder.addNamespace(0, "bar", BlockDuration(12));

			// - add the specified number of blocks up to and including a state change
			for (auto i = 0u; i <= numAliveBlocks; ++i)
				transactionsBuilder.addTransfer(0, 1, Amount(1));

			// - send chain
			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto blocks = builder.asBlockChain(transactionsBuilder);

			test::ExternalSourceConnection connection(context.publicKey());
			test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);
			test::WaitForHeightAndElements(context, Height(3 + numAliveBlocks + 1), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Assert: the cache has the expected balances and namespaces
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 1) }
			});
			test::AssertNamespaceCount(context.localNode(), 3);

			return stateHashes;
		}

		template<typename TTestContext>
		NamespaceStateHashes RunRegisterAndDeactivateNamespaceTest(TTestContext& context) {
			// Act:
			return RunRegisterAndDeactivateNamespaceTest(context, Blocks_Before_Namespace_Deactivate - 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterAndDeactivateNamespace_SingleChainPart) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRegisterAndDeactivateNamespaceTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterAndDeactivateNamespaceWithStateHashEnabled_SingleChainPart) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRegisterAndDeactivateNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonzero(stateHashesPair.second, 2);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[1]); // hash doesn't include new namespaces
	}

	// endregion
}}
