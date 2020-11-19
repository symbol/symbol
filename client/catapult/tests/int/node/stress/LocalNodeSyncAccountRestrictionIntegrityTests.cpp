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

#include "tests/int/node/stress/test/AccountRestrictionTransactionsBuilder.h"
#include "tests/int/node/stress/test/ExpiryTestUtils.h"
#include "tests/int/node/stress/test/LocalNodeSyncIntegrityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeSyncAccountRestrictionIntegrityTests

	namespace {
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;

		// region utils

		using AccountRestrictionStateHashes = std::vector<std::pair<Hash256, Hash256>>;

		Hash256 GetComponentStateHash(const test::PeerLocalNodeTestContext& context) {
			// { AccountState, Namespace, Mosaic, *AccountRestriction* }
			auto subCacheMerkleRoots = context.localNode().cache().createView().calculateStateHash().SubCacheMerkleRoots;
			return subCacheMerkleRoots.empty() ? Hash256() : subCacheMerkleRoots[3];
		}

		void AssertAccountRestrictionCount(const local::LocalNode& localNode, size_t numExpectedRestrictions) {
			// Assert:
			auto numRestrictions = test::GetCounterValue(localNode.counters(), "ACCTREST C");
			EXPECT_EQ(numExpectedRestrictions, numRestrictions);
		}

		template<typename TTestContext>
		std::pair<BlockChainBuilder, std::shared_ptr<model::Block>> PrepareAccountRestriction(
				TTestContext& context,
				const test::Accounts& accounts,
				test::StateHashCalculator& stateHashCalculator,
				AccountRestrictionStateHashes& stateHashes) {
			// Arrange:
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// - prepare restriction
			test::AccountRestrictionTransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addTransfer(0, 2, Amount(1'000'000));
			transactionsBuilder.addTransfer(0, 3, Amount(1'000'000));
			transactionsBuilder.addAccountAddressRestrictionBlock(2, 3);

			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto pAccountRestrictionBlock = utils::UniqueToShared(builder.asSingleBlock(transactionsBuilder));

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo = test::PushEntity(connection, ionet::PacketType::Push_Block, pAccountRestrictionBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(2), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Assert: the cache has expected number of restrictions
			AssertAccountRestrictionCount(context.localNode(), 1);

			return std::make_pair(builder, pAccountRestrictionBlock);
		}

		// endregion

		// region TestFacade

		template<typename TTestContext>
		class TestFacade {
		public:
			explicit TestFacade(TTestContext& context)
					: m_context(context)
					, m_accounts(4)
			{}

		public:
			const auto& accounts() const {
				return m_accounts;
			}

			const auto& stateHashes() const {
				return m_stateHashes;
			}

		public:
			void pushAccountRestriction() {
				// Arrange: push a restriction block
				auto stateHashCalculator = m_context.createStateHashCalculator();
				auto builderBlockPair = PrepareAccountRestriction(m_context, m_accounts, stateHashCalculator, m_stateHashes);

				// Sanity: all restrictions are still present
				AssertAccountRestrictionCount(m_context.localNode(), 1);

				m_allBlocks.emplace_back(builderBlockPair.second);
				m_pActiveBuilder = std::make_unique<BlockChainBuilder>(builderBlockPair.first);
			}

			Blocks createTailBlocks(
					utils::TimeSpan blockInterval,
					const consumer<test::AccountRestrictionTransactionsBuilder&>& addToBuilder) {
				auto stateHashCalculator = m_context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, m_allBlocks);

				test::AccountRestrictionTransactionsBuilder transactionsBuilder(m_accounts);
				addToBuilder(transactionsBuilder);

				auto builder = m_pActiveBuilder->createChainedBuilder(stateHashCalculator);
				builder.setBlockTimeInterval(blockInterval);
				return builder.asBlockChain(transactionsBuilder);
			}

		private:
			TTestContext& m_context;

			test::Accounts m_accounts;
			AccountRestrictionStateHashes m_stateHashes;
			std::unique_ptr<BlockChainBuilder> m_pActiveBuilder;
			std::vector<std::shared_ptr<model::Block>> m_allBlocks;
		};

		// endregion
	}

	// region restriction (add)

	namespace {
		template<typename TTestContext>
		AccountRestrictionStateHashes RunAddAccountRestrictionTest(TTestContext& context) {
			// Arrange:
			AccountRestrictionStateHashes stateHashes;
			test::Accounts accounts(4);
			auto stateHashCalculator = context.createStateHashCalculator();

			// Act + Assert:
			PrepareAccountRestriction(context, accounts, stateHashCalculator, stateHashes);

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanAddAccountRestriction) {
		// Arrange:
		test::StateHashDisabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunAddAccountRestrictionTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanAddAccountRestrictionWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunAddAccountRestrictionTest(context));

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - restriction cache merkle root is only nonzero when restriction is added
		ASSERT_EQ(2u, stateHashesPair.second.size());
		EXPECT_EQ(Hash256(), stateHashesPair.second[0]);
		EXPECT_NE(Hash256(), stateHashesPair.second[1]);
	}

	// endregion

	// region restriction (remove)

	namespace {
		template<typename TTestContext>
		AccountRestrictionStateHashes RunRemoveAccountRestrictionTest(TTestContext& context) {
			// Arrange: create a restriction
			TestFacade<TTestContext> facade(context);
			facade.pushAccountRestriction();

			// - prepare block that will remove the restriction
			auto nextBlocks = facade.createTailBlocks(utils::TimeSpan::FromSeconds(60), [](auto& transactionsBuilder) {
				transactionsBuilder.delAccountAddressRestrictionBlock(2, 3);
			});

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, nextBlocks);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(3), 2, 1);

			// Assert: the cache has no restrictions
			AssertAccountRestrictionCount(context.localNode(), 0);

			auto stateHashes = facade.stateHashes();
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));
			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanRemoveAccountRestriction) {
		// Arrange:
		test::StateHashDisabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRemoveAccountRestrictionTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 3);
	}

	NO_STRESS_TEST(TEST_CLASS, CanRemoveAccountRestrictionWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunRemoveAccountRestrictionTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 3);
		test::AssertUnique(stateHashesPair.first);

		// - restriction cache merkle root is initially zero (no restrictions in nemesis)
		ASSERT_EQ(3u, stateHashesPair.second.size());
		EXPECT_EQ(Hash256(), stateHashesPair.second[0]);

		// - restriction is active at 1
		EXPECT_NE(stateHashesPair.second[0], stateHashesPair.second[1]);

		// - restriction is finally removed
		EXPECT_EQ(stateHashesPair.second[0], stateHashesPair.second[2]);
	}

	// endregion

	// region restriction (add + remove) [single chain part]

	namespace {
		template<typename TTestContext>
		AccountRestrictionStateHashes RunAddAndRemoveAccountRestrictionTest(TTestContext& context) {
			// Arrange:
			AccountRestrictionStateHashes stateHashes;
			test::Accounts accounts(4);
			auto stateHashCalculator = context.createStateHashCalculator();

			// - wait for boot
			test::WaitForBoot(context);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// - prepare restriction chain with add and remove
			test::AccountRestrictionTransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addTransfer(0, 2, Amount(1'000'000));
			transactionsBuilder.addTransfer(0, 3, Amount(1'000'000));
			transactionsBuilder.addAccountAddressRestrictionBlock(2, 3);
			transactionsBuilder.delAccountAddressRestrictionBlock(2, 3);

			// - send chain
			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto blocks = builder.asBlockChain(transactionsBuilder);

			test::ExternalSourceConnection connection(context.publicKey());
			test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);
			test::WaitForHeightAndElements(context, Height(5), 1, 1);
			stateHashes.emplace_back(GetStateHash(context), GetComponentStateHash(context));

			// Assert: the cache has no restrictions
			AssertAccountRestrictionCount(context.localNode(), 0);

			return stateHashes;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CanAddAndRemoveAccountRestriction_SingleChainPart) {
		// Arrange:
		test::StateHashDisabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunAddAndRemoveAccountRestrictionTest(context));

		// Assert:
		test::AssertAllZero(stateHashesPair, 2);
	}

	NO_STRESS_TEST(TEST_CLASS, CanAddAndRemoveAccountRestrictionWithStateHashEnabled_SingleChainPart) {
		// Arrange:
		test::StateHashEnabledTestContext context(test::NonNemesisTransactionPlugins::Restriction_Account);

		// Act + Assert:
		auto stateHashesPair = test::Unzip(RunAddAndRemoveAccountRestrictionTest(context));

		// Assert: all state hashes are nonzero (since importance is recalculated every block none of the hashes are the same)
		test::AssertAllNonzero(stateHashesPair.first, 2);
		test::AssertUnique(stateHashesPair.first);

		// - all restriction merkle roots are zero because restriction is only active inbetween
		test::AssertAllZero(stateHashesPair.second, 2);
	}

	// endregion
}}
