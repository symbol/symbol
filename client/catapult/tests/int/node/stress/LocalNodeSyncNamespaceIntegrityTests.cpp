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

#include "tests/int/node/stress/test/LocalNodeSyncIntegrityTestUtils.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeSyncNamespaceIntegrityTests

	namespace {
		using Accounts = test::Accounts;
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;

		// region utils

		using NamespaceStateHashes = std::vector<std::pair<Hash256, Hash256>>;

		Hash256 GetNamespaceStateHash(const test::PeerLocalNodeTestContext& context) {
			auto subCacheMerkleRoots = context.localNode().cache().createView().calculateStateHash().SubCacheMerkleRoots;
			return subCacheMerkleRoots.empty() ? Hash256() : subCacheMerkleRoots[1]; // namespace state hash is second
		}

		template<typename T>
		std::pair<std::vector<T>, std::vector<T>> Unzip(const std::vector<std::pair<T, T>>& pairs) {
			std::pair<std::vector<T>, std::vector<T>> result;
			for (const auto& pair : pairs) {
				result.first.emplace_back(pair.first);
				result.second.emplace_back(pair.second);
			}

			return result;
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
				const Accounts& accounts,
				test::StateHashCalculator& stateHashCalculator,
				NamespaceStateHashes& stateHashes) {
			// Arrange:
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity:
			AssertBooted(context);

			// - prepare namespace registrations
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addNamespace(0, "foo", BlockDuration(12));
			builder.addNamespace(0, "bar", BlockDuration(12));
			auto pNamespaceBlock = utils::UniqueToShared(builder.asSingleBlock());

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pNamespaceBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity: the cache has expected namespaces
			test::AssertNamespaceCount(context.localNode(), 3);

			return std::make_pair(builder, std::move(pNamespaceBlock));
		}

		struct TransferBlocksResult {
			BlockChainBuilder Builder;
			Blocks AllBlocks;
			uint32_t NumAliveChains;
		};

		template<typename TTestContext>
		TransferBlocksResult PushTransferBlocks(
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

				test::WaitForHeightAndElements(context, Height(2 + numTotalBlocks - numRemainingBlocks), 1 + numAliveChains, 1);
				if (0 == numRemainingBlocks)
					break;

				builder = builder.createChainedBuilder();
			}

			return TransferBlocksResult{ builder, allBlocks, numAliveChains };
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
			AssertBooted(context);

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
			test::WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has expected namespaces
			test::AssertNamespaceCount(context.localNode(), 3);

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunRegisterNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 2);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunRegisterNamespaceTest(context));

		// Assert: all state hashes are nonzero
		test::AssertAllNonZero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 2);
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
			// Arrange:
			NamespaceStateHashes stateHashes;
			Accounts accounts(2);
			auto stateHashCalculator = context.createStateHashCalculator();
			auto builderBlockPair = PrepareTwoRootNamespaces(context, accounts, stateHashCalculator, stateHashes);
			auto& builder = builderBlockPair.first;

			// - add the specified number of blocks up to a state change
			test::ExternalSourceConnection connection;
			auto builder2 = builder.createChainedBuilder();
			auto transferBlocksResult = PushTransferBlocks(context, connection, builder2, numAliveBlocks);
			auto numAliveChains = transferBlocksResult.NumAliveChains;
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity: all namespaces are still present
			test::AssertNamespaceCount(context.localNode(), 3);

			// - prepare a block that triggers a state change
			auto builder3 = transferBlocksResult.Builder.createChainedBuilder();
			builder3.addTransfer(0, 1, Amount(1));
			auto pTailBlock = utils::UniqueToShared(builder3.asSingleBlock());

			// Act:
			auto pIo1 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 1), 1 + numAliveChains + 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has the expected balances and namespaces
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 1) }
			});
			test::AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

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
		auto stateHashesPair = Unzip(RunExpireNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonZero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 4);
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
			return RunNamespaceStateChangeTest(context, Blocks_Before_Namespace_Prune - 1, 1);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespace) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonZero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]); // hash does not include new namespaces (expired)
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[3]); // hash does not include new namespaces (pruned)
	}

	// endregion

	// region namespace (register + deactivate) :: single chain part

	namespace {
		// namespace is deactivated after the sum of the following
		//   (1) namespace duration ==> 12
		//   (2) grace period ========> 1hr of blocks with 20s target time
		constexpr auto Blocks_Before_Namespace_Deactivate = static_cast<uint32_t>(12 + (utils::TimeSpan::FromHours(1).seconds() / 20));

		template<typename TTestContext>
		NamespaceStateHashes RunRegisterAndDeactivateNamespaceTest(TTestContext& context, size_t numAliveBlocks) {
			// Arrange:
			NamespaceStateHashes stateHashes;
			Accounts accounts(2);
			auto stateHashCalculator = context.createStateHashCalculator();

			// *** customization of PrepareTwoRootNamespaces ***
			// - create namespace registration block
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Sanity:
			AssertBooted(context);

			// - prepare namespace registrations
			BlockChainBuilder builder(accounts, stateHashCalculator);
			builder.addNamespace(0, "foo", BlockDuration(12));
			builder.addNamespace(0, "bar", BlockDuration(12));
			auto pNamespaceBlock = utils::UniqueToShared(builder.asSingleBlock());

			// - add the specified number of blocks up to and including a state change
			test::ExternalSourceConnection connection;
			auto builder2 = builder.createChainedBuilder();

			for (auto i = 0u; i <= numAliveBlocks; ++i)
				builder2.addTransfer(0, 1, Amount(1));

			auto blocks = builder2.asBlockChain();
			blocks.insert(blocks.begin(), pNamespaceBlock);
			test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);
			test::WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 1), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

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
		auto stateHashesPair = Unzip(RunRegisterAndDeactivateNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 2);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRegisterAndDeactivateNamespaceWithStateHashEnabled_SingleChainPart) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunRegisterAndDeactivateNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonZero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 2);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[1]); // hash doesn't include new namespaces
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
				auto transferBlocksResult = PushTransferBlocks(context, connection, builder2, numAliveBlocks);
				numAliveChains = transferBlocksResult.NumAliveChains;
				stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

				// Sanity: all namespaces are still present
				test::AssertNamespaceCount(context.localNode(), 3);

				allBlocks.emplace_back(builderBlockPair.second);
				allBlocks.insert(allBlocks.end(), transferBlocksResult.AllBlocks.cbegin(), transferBlocksResult.AllBlocks.cend());
				pBuilder = std::make_unique<BlockChainBuilder>(builder2);
			}

			// - prepare a block that triggers a state change
			std::shared_ptr<model::Block> pExpiryBlock1;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, allBlocks);

				auto builder3 = pBuilder->createChainedBuilder(stateHashCalculator);
				builder3.addTransfer(0, 1, Amount(1));
				pExpiryBlock1 = utils::UniqueToShared(builder3.asSingleBlock());
			}

			// - prepare two blocks that triggers a state change
			Blocks expiryBlocks2;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, allBlocks);

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
			test::WaitForHeightAndElements(context, Height(2 + numAliveBlocks + 2), 1 + numAliveChains + 2, 2);
			stateHashes.emplace_back(GetStateHash(context), GetNamespaceStateHash(context));

			// Assert: the cache has the expected balances and namespaces
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(numAliveBlocks + 2) }
			});
			test::AssertNamespaceCount(context.localNode(), numExpectedNamespaces);

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
		auto stateHashesPair = Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanExpireAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunExpireAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonZero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 4);
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
		auto stateHashesPair = Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashesPair.first, 4);

		// - all namespace cache merkle roots are zero
		test::AssertAllZero(stateHashesPair.second, 4);
	}

	NO_STRESS_TEST(TEST_CLASS, CanPruneAndRollbackNamespaceWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashesPair = Unzip(RunPruneAndRollbackNamespaceTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonZero(stateHashesPair.first, 4);
		test::AssertUnique(stateHashesPair.first);

		// - all namespace cache merkle roots are nonzero
		test::AssertAllNonZero(stateHashesPair.second, 4);
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]); // hash does not include new namespaces (expired)
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[3]); // hash does not include new namespaces (pruned)
	}

	// endregion
}}
