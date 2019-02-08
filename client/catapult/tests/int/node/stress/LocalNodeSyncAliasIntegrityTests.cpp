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

#define TEST_CLASS LocalNodeSyncAliasIntegrityTests

	namespace {
		using Accounts = test::Accounts;
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;
	}

	// region alias application

	namespace {
		template<typename TTestContext>
		std::vector<Hash256> RunApplyAliasTest(TTestContext& context) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			test::WaitForBoot(context);
			stateHashes.emplace_back(test::GetStateHash(context));

			// Sanity:
			EXPECT_EQ(Height(1), context.height());

			// - prepare transactions
			Accounts accounts(3);
			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder(accounts, stateHashCalculator);

			// - make transfers to the accounts to be aliased so that they're in the account state cache
			builder.addTransfer(0, 1, Amount(1));
			builder.addTransfer(0, 2, Amount(1));

			// -  make root namespaces and assign address aliases to each one
			builder.addNamespace(0, "foo", BlockDuration(12), 1); // root "foo" namespace with alias for account 1
			builder.addNamespace(0, "bar", BlockDuration(12), 2); // root "bar" namespace with alias for account 2

			// - send direct
			builder.addTransfer(0, 1, Amount(1'000'000));
			builder.addTransfer(0, 2, Amount(900'000));

			// - send via alias
			builder.addTransfer(0, "foo", Amount(700'000));
			builder.addTransfer(0, "bar", Amount(400'000));
			auto pBlock = utils::UniqueToShared(builder.asSingleBlock());

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(test::GetStateHash(context));

			// Assert: the cache has expected namespaces
			test::AssertNamespaceCount(context.localNode(), 3);

			// - the cache has expected balances
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(1'700'001) },
				{ 2, Amount(1'300'001) }
			});

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanApplyAlias) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyAliasTest(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanApplyAliasWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyAliasTest(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonZero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region alias application + rollback

	namespace {
		template<typename TTestContext>
		std::shared_ptr<model::Block> CreateBlockWithTwoAliasesAndTransfers(
				const TTestContext& context,
				const BlockChainBuilder& builder,
				const Blocks& seedBlocks,
				Timestamp blockTimeInterval,
				size_t aliasId1,
				size_t aliasId2) {
			auto stateHashCalculator = context.createStateHashCalculator();
			SeedStateHashCalculator(stateHashCalculator, seedBlocks);
			auto builder2 = builder.createChainedBuilder(stateHashCalculator);
			builder2.setBlockTimeInterval(blockTimeInterval);

			// -  make root namespaces and assign address aliases to each one
			builder2.addNamespace(0, "foo", BlockDuration(12), aliasId1); // root "foo" namespace with alias for account `aliasId1`
			builder2.addNamespace(0, "bar", BlockDuration(12), aliasId2); // root "bar" namespace with alias for account `aliasId2`

			// - send via alias
			builder2.addTransfer(0, "foo", Amount(700'000));
			builder2.addTransfer(0, "bar", Amount(400'000));
			return utils::UniqueToShared(builder2.asSingleBlock());
		}

		template<typename TTestContext>
		std::vector<Hash256> RunRollbackAliasTest(TTestContext& context) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			Accounts accounts(4);

			// - make transfers to the accounts to be aliased so that they're in the account state cache
			std::unique_ptr<BlockChainBuilder> pBuilder1;
			Blocks seedBlocks;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				BlockChainBuilder builder(accounts, stateHashCalculator);
				builder.addTransfer(0, 1, Amount(1));
				builder.addTransfer(0, 2, Amount(1));
				builder.addTransfer(0, 3, Amount(1));
				auto pBlock = utils::UniqueToShared(builder.asSingleBlock());

				test::ExternalSourceConnection connection;
				auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pBlock);

				// - wait for the chain height to change and for all height readers to disconnect
				test::WaitForHeightAndElements(context, Height(2), 1, 1);
				stateHashes.emplace_back(test::GetStateHash(context));

				pBuilder1 = std::make_unique<BlockChainBuilder>(builder);
				seedBlocks = { pBlock };
			}

			// - create two blocks with same aliases pointing to different accounts where (better) second block will yield better chain
			auto pTailBlock1 = CreateBlockWithTwoAliasesAndTransfers(context, *pBuilder1, seedBlocks, Timestamp(60'000), 1, 2);
			auto pTailBlock2 = CreateBlockWithTwoAliasesAndTransfers(context, *pBuilder1, seedBlocks, Timestamp(58'000), 2, 3);

			// Act:
			test::ExternalSourceConnection connection;
			auto pIo1 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock1);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock2);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + seedBlocks.size() + 1), 3, 2);
			stateHashes.emplace_back(test::GetStateHash(context));

			// Assert: the cache has expected namespaces
			test::AssertNamespaceCount(context.localNode(), 3);

			// - the cache has expected balances (transfers sent using aliases set up in pTailBlock2)
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
				{ 1, Amount(1) },
				{ 2, Amount(700'001) },
				{ 3, Amount(400'001) }
			});

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRollbackAlias) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackAliasTest(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRollbackAliasWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackAliasTest(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonZero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion
}}
