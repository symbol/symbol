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

#include "sdk/src/extensions/BlockExtensions.h"
#include "tests/int/node/stress/test/LocalNodeSyncIntegrityTestUtils.h"
#include "tests/int/node/stress/test/TransactionsBuilder.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeSyncTransferIntegrityTests

	namespace {
		using BlockChainBuilder = test::BlockChainBuilder;
		using Blocks = BlockChainBuilder::Blocks;

		// region traits

		struct SingleBlockTraits {
			static auto GetBlocks(BlockChainBuilder& builder, const test::TransactionsGenerator& transactionsGenerator) {
				return Blocks{ utils::UniqueToShared(builder.asSingleBlock(transactionsGenerator)) };
			}
		};

		struct MultiBlockTraits {
			static auto GetBlocks(BlockChainBuilder& builder, const test::TransactionsGenerator& transactionsGenerator) {
				return builder.asBlockChain(transactionsGenerator);
			}
		};

#define SINGLE_MULTI_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_SingleBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleBlockTraits>(); } \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_BlockChain) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultiBlockTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion

		// region utils

		void ResignBlock(model::Block& block) {
			for (const auto* pPrivateKeyString : test::Test_Network_Private_Keys) {
				auto keyPair = crypto::KeyPair::FromString(pPrivateKeyString);
				if (keyPair.publicKey() == block.SignerPublicKey) {
					extensions::BlockExtensions(test::GetNemesisGenerationHashSeed()).signFullBlock(keyPair, block);
					return;
				}
			}

			CATAPULT_THROW_RUNTIME_ERROR("unable to find block signer among test private keys");
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
			stateHashes.emplace_back(GetStateHash(context));

			// Sanity:
			EXPECT_EQ(Height(1), context.height());

			// - prepare transfers (all transfers are dependent on previous transfer)
			test::Accounts accounts(6);
			test::TransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addTransfer(0, 1, Amount(1'000'000));
			transactionsBuilder.addTransfer(1, 2, Amount(900'000));
			transactionsBuilder.addTransfer(2, 3, Amount(700'000));
			transactionsBuilder.addTransfer(3, 4, Amount(400'000));
			transactionsBuilder.addTransfer(4, 5, Amount(50'000));

			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto blocks = TTraits::GetBlocks(builder, transactionsBuilder);

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + blocks.size()), 1, 1);
			stateHashes.emplace_back(GetStateHash(context));

			// Assert: the cache has expected balances
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
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
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (success)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::pair<BlockChainBuilder, Blocks> PrepareFiveChainedTransfers(
				TTestContext& context,
				const test::Accounts& accounts,
				test::StateHashCalculator& stateHashCalculator) {
			// Arrange:
			test::WaitForBoot(context);

			// Sanity:
			EXPECT_EQ(Height(1), context.height());

			// - prepare transfers (all transfers are dependent on previous transfer)
			test::TransactionsBuilder transactionsBuilder(accounts);
			transactionsBuilder.addTransfer(0, 1, Amount(1'000'000));
			transactionsBuilder.addTransfer(1, 2, Amount(900'000));
			transactionsBuilder.addTransfer(2, 3, Amount(700'000));
			transactionsBuilder.addTransfer(3, 4, Amount(400'000));
			transactionsBuilder.addTransfer(4, 5, Amount(50'000));

			BlockChainBuilder builder(accounts, stateHashCalculator);
			auto blocks = TTraits::GetBlocks(builder, transactionsBuilder);

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + blocks.size()), 1, 1);
			return std::make_pair(builder, blocks);
		}

		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRollbackTest(TTestContext& context) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			test::Accounts accounts(6);
			{
				// - always use SingleBlockTraits because a push can rollback at most one block
				auto stateHashCalculator = context.createStateHashCalculator();
				PrepareFiveChainedTransfers<SingleBlockTraits>(context, accounts, stateHashCalculator);
				stateHashes.emplace_back(GetStateHash(context));
			}

			// - prepare transfers (all are from nemesis)
			test::TransactionsBuilder transactionsBuilder1(accounts);
			transactionsBuilder1.addTransfer(0, 1, Amount(1'000'000));
			transactionsBuilder1.addTransfer(0, 2, Amount(900'000));
			transactionsBuilder1.addTransfer(0, 3, Amount(700'000));
			transactionsBuilder1.addTransfer(0, 4, Amount(400'000));
			transactionsBuilder1.addTransfer(0, 5, Amount(50'000));

			auto stateHashCalculator = context.createStateHashCalculator();
			BlockChainBuilder builder1(accounts, stateHashCalculator);
			builder1.setBlockTimeInterval(utils::TimeSpan::FromSeconds(58)); // better block time will yield better chain
			auto blocks = TTraits::GetBlocks(builder1, transactionsBuilder1);

			// - prepare a transfer that can only attach to rollback case
			test::TransactionsBuilder transactionsBuilder2(accounts);
			transactionsBuilder2.addTransfer(2, 4, Amount(350'000));

			auto builder2 = builder1.createChainedBuilder();
			auto pTailBlock = utils::UniqueToShared(builder2.asSingleBlock(transactionsBuilder2));

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + blocks.size() + 1), 3, 2);
			stateHashes.emplace_back(GetStateHash(context));

			// Assert: the cache has expected balances
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
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
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application (validation failure)

	namespace {
		template<typename TTraits, typename TTestContext, typename TGenerateInvalidBlocks>
		std::vector<Hash256> RunRejectInvalidApplyTest(TTestContext& context, TGenerateInvalidBlocks generateInvalidBlocks) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			test::Accounts accounts(6);
			std::unique_ptr<BlockChainBuilder> pBuilder1;
			Blocks seedBlocks;
			{
				// - seed the chain with initial blocks
				auto stateHashCalculator = context.createStateHashCalculator();
				auto builderBlocksPair = PrepareFiveChainedTransfers<TTraits>(context, accounts, stateHashCalculator);
				pBuilder1 = std::make_unique<BlockChainBuilder>(builderBlocksPair.first);
				seedBlocks = builderBlocksPair.second;
				stateHashes.emplace_back(GetStateHash(context));
			}

			Blocks invalidBlocks;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare invalid blocks
				auto builder2 = pBuilder1->createChainedBuilder(stateHashCalculator);
				invalidBlocks = generateInvalidBlocks(accounts, builder2);
			}

			std::shared_ptr<model::Block> pTailBlock;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare a transfer that can only attach to initial blocks
				test::TransactionsBuilder transactionsBuilder3(accounts);
				transactionsBuilder3.addTransfer(1, 2, Amount(91'000));

				auto builder3 = pBuilder1->createChainedBuilder(stateHashCalculator);
				pTailBlock = utils::UniqueToShared(builder3.asSingleBlock(transactionsBuilder3));
			}

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, invalidBlocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + seedBlocks.size() + 1), 3, 2);
			stateHashes.emplace_back(GetStateHash(context));

			// Assert: the cache has expected balances
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
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
			return RunRejectInvalidApplyTest<TTraits>(context, [](const auto& accounts, auto& builder) {
				// Arrange: prepare three transfers, where second is invalid
				test::TransactionsBuilder transactionsBuilder(accounts);
				transactionsBuilder.addTransfer(1, 5, Amount(90'000));
				transactionsBuilder.addTransfer(2, 5, Amount(200'001));
				transactionsBuilder.addTransfer(3, 5, Amount(80'000));
				return TTraits::GetBlocks(builder, transactionsBuilder);
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationApplyTransactions) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application (state hash failure)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidStateHashApplyTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidApplyTest<TTraits>(context, [](const auto& accounts, auto& builder) {
				// Arrange: prepare three valid transfers
				test::TransactionsBuilder transactionsBuilder(accounts);
				transactionsBuilder.addTransfer(1, 5, Amount(90'000));
				transactionsBuilder.addTransfer(2, 5, Amount(80'000));
				transactionsBuilder.addTransfer(3, 5, Amount(80'000));
				auto blocks = TTraits::GetBlocks(builder, transactionsBuilder);

				// - corrupt state hash of last block
				test::FillWithRandomData(blocks.back()->StateHash);
				ResignBlock(*blocks.back());
				return blocks;
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashApplyTransactions) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashApplyTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashApplyTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashApplyTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (validation failure)

	namespace {
		template<typename TTraits, typename TTestContext, typename TGenerateInvalidBlocks>
		std::vector<Hash256> RunRejectInvalidRollbackTest(TTestContext& context, TGenerateInvalidBlocks generateInvalidBlocks) {
			// Arrange:
			std::vector<Hash256> stateHashes;
			test::Accounts accounts(6);
			std::unique_ptr<BlockChainBuilder> pBuilder1;
			Blocks seedBlocks;
			{
				// - seed the chain with initial blocks
				// - always use SingleBlockTraits because a push can rollback at most one block
				auto stateHashCalculator = context.createStateHashCalculator();
				auto builderBlocksPair = PrepareFiveChainedTransfers<SingleBlockTraits>(context, accounts, stateHashCalculator);
				pBuilder1 = std::make_unique<BlockChainBuilder>(builderBlocksPair.first);
				seedBlocks = builderBlocksPair.second;
				stateHashes.emplace_back(GetStateHash(context));
			}

			Blocks invalidBlocks;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				BlockChainBuilder builder2(accounts, stateHashCalculator);

				// - prepare invalid blocks
				builder2.setBlockTimeInterval(utils::TimeSpan::FromSeconds(58)); // better block time will yield better chain
				invalidBlocks = generateInvalidBlocks(accounts, builder2);
			}

			std::shared_ptr<model::Block> pTailBlock;
			{
				auto stateHashCalculator = context.createStateHashCalculator();
				test::SeedStateHashCalculator(stateHashCalculator, seedBlocks);

				// - prepare a transfer that can only attach to initial blocks
				test::TransactionsBuilder transactionsBuilder3(accounts);
				transactionsBuilder3.addTransfer(1, 2, Amount(91'000));

				auto builder3 = pBuilder1->createChainedBuilder(stateHashCalculator);
				pTailBlock = utils::UniqueToShared(builder3.asSingleBlock(transactionsBuilder3));
			}

			// Act:
			test::ExternalSourceConnection connection(context.publicKey());
			auto pIo1 = test::PushEntities(connection, ionet::PacketType::Push_Block, invalidBlocks);
			auto pIo2 = test::PushEntity(connection, ionet::PacketType::Push_Block, pTailBlock);

			// - wait for the chain height to change and for all height readers to disconnect
			test::WaitForHeightAndElements(context, Height(1 + seedBlocks.size() + 1), 3, 2);
			stateHashes.emplace_back(GetStateHash(context));

			// Assert: the cache has expected balances
			test::AssertCurrencyBalances(accounts, context.localNode().cache(), {
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
			return RunRejectInvalidRollbackTest<TTraits>(context, [](const auto& accounts, auto& builder) {
				// Arrange: prepare five transfers, where third is invalid
				test::TransactionsBuilder transactionsBuilder(accounts);
				transactionsBuilder.addTransfer(0, 1, Amount(10'000));
				transactionsBuilder.addTransfer(0, 2, Amount(900'000));
				transactionsBuilder.addTransfer(2, 3, Amount(900'001));
				transactionsBuilder.addTransfer(0, 4, Amount(400'000));
				transactionsBuilder.addTransfer(0, 5, Amount(50'000));
				return TTraits::GetBlocks(builder, transactionsBuilder);
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationRollbackTransactions) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidValidationRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidValidationRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion

	// region transfer application + rollback (state hash failure)

	namespace {
		template<typename TTraits, typename TTestContext>
		std::vector<Hash256> RunRejectInvalidStateHashRollbackTest(TTestContext& context) {
			// Act + Assert:
			return RunRejectInvalidRollbackTest<TTraits>(context, [](const auto& accounts, auto& builder) {
				// Arrange: prepare five valid transfers
				test::TransactionsBuilder transactionsBuilder(accounts);
				transactionsBuilder.addTransfer(0, 1, Amount(10'000));
				transactionsBuilder.addTransfer(0, 2, Amount(900'000));
				transactionsBuilder.addTransfer(0, 3, Amount(900'001));
				transactionsBuilder.addTransfer(0, 4, Amount(400'000));
				transactionsBuilder.addTransfer(0, 5, Amount(50'000));
				auto blocks = TTraits::GetBlocks(builder, transactionsBuilder);

				// - corrupt state hash of last block
				test::FillWithRandomData(blocks.back()->StateHash);
				ResignBlock(*blocks.back());
				return blocks;
			});
		}
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashRollbackTransactions) {
		// Arrange:
		test::StateHashDisabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashRollbackTest<TTraits>(context);

		// Assert: all state hashes are zero
		test::AssertAllZero(stateHashes, 2);
	}

	SINGLE_MULTI_BASED_TEST(CanRejectInvalidStateHashRollbackTransactionsWithStateHashEnabled) {
		// Arrange:
		test::StateHashEnabledTestContext context;

		// Act + Assert:
		auto stateHashes = RunRejectInvalidStateHashRollbackTest<TTraits>(context);

		// Assert: all state hashes are nonzero
		test::AssertAllNonzero(stateHashes, 2);
		test::AssertUnique(stateHashes);
	}

	// endregion
}}
