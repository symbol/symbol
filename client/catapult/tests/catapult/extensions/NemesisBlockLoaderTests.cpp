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

#include "catapult/extensions/NemesisBlockLoader.h"
#include "plugins/coresystem/src/observers/Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BalanceTransfers.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NemesisBlockLoaderTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		// loader doesn't do anything with currency mosaic, but it does with harvesting mosaic
		// harvesting mosaic transfers need to be tested for correct behavior
		// currency mosaic is effectively handled the same as any "other" mosaic
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		// region create/set nemesis block

		struct BlockSignerPair {
			std::unique_ptr<crypto::KeyPair> pSigner;
			std::unique_ptr<model::Block> pBlock;
		};

		BlockSignerPair CreateNemesisBlock(const std::vector<test::BalanceTransfers>& transferGroups) {
			BlockSignerPair blockSignerPair;
			blockSignerPair.pSigner = std::make_unique<crypto::KeyPair>(test::GenerateKeyPair());
			auto nemesisPublicKey = blockSignerPair.pSigner->publicKey();
			model::Transactions transactions;
			for (const auto& transfers : transferGroups) {
				// since XOR mosaic resolver is used in tests, unresolve all transfers so that they can be roundtripped
				// via resolvers during processing
				std::vector<model::UnresolvedMosaic> unresolvedTransfers;
				for (const auto& transfer : transfers)
					unresolvedTransfers.push_back({ test::UnresolveXor(transfer.MosaicId), transfer.Amount });

				auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), unresolvedTransfers);
				pTransaction->Signer = nemesisPublicKey;
				transactions.push_back(std::move(pTransaction));
			}

			blockSignerPair.pBlock = CreateBlock(model::PreviousBlockContext(), Network_Identifier, nemesisPublicKey, transactions);
			return blockSignerPair;
		}

		enum class NemesisBlockModification { None, Public_Key, Generation_Hash, Signature };

		void SetNemesisBlock(
				io::BlockStorageCache& storage,
				const BlockSignerPair& blockSignerPair,
				const model::NetworkInfo& network,
				NemesisBlockModification modification) {
			// note that this trick only works for MockMemoryBlockStorage, which allows the nemesis block to be dropped
			auto pNemesisBlockElement = storage.view().loadBlockElement(Height(1));
			auto storageModifier = storage.modifier();
			storageModifier.dropBlocksAfter(Height(0));

			// copy and prepare the block
			auto pModifiedBlock = test::CopyBlock(*blockSignerPair.pBlock);

			// 1. modify the block signer if requested
			if (NemesisBlockModification::Public_Key == modification)
				test::FillWithRandomData(pModifiedBlock->Signer);
			else
				pModifiedBlock->Signer = network.PublicKey;

			// 2. modify the generation hash if requested
			auto modifiedNemesisBlockElement = test::BlockToBlockElement(*pModifiedBlock);
			if (NemesisBlockModification::Generation_Hash == modification)
				test::FillWithRandomData(modifiedNemesisBlockElement.GenerationHash);
			else
				modifiedNemesisBlockElement.GenerationHash = network.GenerationHash;

			// 3. modify the block signature if requested
			if (NemesisBlockModification::Signature == modification)
				test::FillWithRandomData(pModifiedBlock->Signature);
			else
				test::SignBlock(*blockSignerPair.pSigner, *pModifiedBlock);

			storageModifier.saveBlock(modifiedNemesisBlockElement);
		}

		// endregion

		// region utils

		model::Mosaic MakeHarvestingMosaic(Amount amount) {
			return { Harvesting_Mosaic_Id, amount };
		}

		model::Mosaic MakeHarvestingMosaic(Amount::ValueType amount) {
			return MakeHarvestingMosaic(Amount(amount));
		}

		model::BlockChainConfiguration CreateDefaultConfiguration(const model::Block& nemesisBlock, Importance totalChainImportance) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.Network.PublicKey = nemesisBlock.Signer;
			test::FillWithRandomData(config.Network.GenerationHash);
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.TotalChainImportance = totalChainImportance;
			return config;
		}

		plugins::PluginManager CreatePluginManager() {
			// enable Publish_Transfers (MockTransaction Publish XORs recipient address, so XOR address resolver is required
			// for proper roundtripping or else test will fail)
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			plugins::PluginManager manager(config, plugins::StorageConfiguration());
			manager.addTransactionSupport(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));

			manager.addMosaicResolver([](const auto&, const auto& unresolved, auto& resolved) {
				resolved = test::CreateResolverContextXor().resolve(unresolved);
				return true;
			});
			manager.addAddressResolver([](const auto&, const auto& unresolved, auto& resolved) {
				resolved = test::CreateResolverContextXor().resolve(unresolved);
				return true;
			});
			return manager;
		}

		std::unique_ptr<const observers::NotificationObserver> CreateObserver() {
			// use real coresystem observers to create accounts, update balances and add harvest receipt
			observers::DemuxObserverBuilder builder;
			builder
				.add(observers::CreateAccountAddressObserver())
				.add(observers::CreateAccountPublicKeyObserver())
				.add(observers::CreateBalanceTransferObserver())
				.add(observers::CreateHarvestFeeObserver(Harvesting_Mosaic_Id));
			return builder.build();
		}

		const Key& GetTransactionRecipient(const model::Block& block, size_t index) {
			auto transactions = block.Transactions();
			auto iter = transactions.cbegin();
			for (auto i = 0u; i < index; ++i, ++iter);
			return static_cast<const mocks::MockTransaction&>(*iter).Recipient;
		}

		// endregion

		// region generic tests

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const BlockSignerPair& nemesisBlockSignerPair,
				Importance totalChainImportance,
				StateHashVerification stateHashVerification,
				TAssertAccountStateCache assertAccountStateCache) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(*nemesisBlockSignerPair.pBlock, totalChainImportance);
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, NemesisBlockModification::None);

			// - create the publisher, observer and loader
			auto pluginManager = CreatePluginManager();
			auto cacheDelta = state.ref().Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

			// Act:
			TTraits::Execute(loader, state.ref(), stateHashVerification);

			// Assert:
			TTraits::Assert(cacheDelta, state.ref(), assertAccountStateCache);
		}

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const BlockSignerPair& nemesisBlockSignerPair,
				Importance totalChainImportance,
				TAssertAccountStateCache assertAccountStateCache) {
			RunLoadNemesisBlockTest<TTraits>(
					nemesisBlockSignerPair,
					totalChainImportance,
					StateHashVerification::Enabled,
					assertAccountStateCache);
		}

		enum class NemesisBlockVerifyOptions { None, State, Receipts };

		template<typename TTraits, typename TException = catapult_invalid_argument>
		void AssertLoadNemesisBlockFailure(
				const BlockSignerPair& nemesisBlockSignerPair,
				Importance totalChainImportance,
				NemesisBlockModification modification = NemesisBlockModification::None,
				NemesisBlockVerifyOptions verifyOptions = NemesisBlockVerifyOptions::None) {
			// Arrange:
			bool enableVerifiableState = NemesisBlockVerifyOptions::State == verifyOptions;

			// - create the state
			auto config = CreateDefaultConfiguration(*nemesisBlockSignerPair.pBlock, totalChainImportance);
			config.ShouldEnableVerifiableReceipts = NemesisBlockVerifyOptions::Receipts == verifyOptions;

			auto cacheConfig = cache::CacheConfiguration();
			test::TempDirectoryGuard dbDirGuard;
			if (enableVerifiableState)
				cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

			auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
			test::LocalNodeTestState state(config, "", std::move(cache));
			SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, modification);

			// - create the publisher, observer and loader
			auto pluginManager = CreatePluginManager();
			auto cacheDelta = state.ref().Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

			// Act + Assert:
			EXPECT_THROW(TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled), TException);

			// Sanity:
			auto cacheStateHash = state.cref().Cache.createView().calculateStateHash().StateHash;
			if (enableVerifiableState)
				EXPECT_NE(Hash256(), cacheStateHash);
			else
				EXPECT_EQ(Hash256(), cacheStateHash);
		}

		// endregion
	}

	// region traits

	namespace {
		struct ExecuteTraits {
			static void Execute(
					NemesisBlockLoader& loader,
					const LocalNodeStateRef& stateRef,
					StateHashVerification stateHashVerification) {
				loader.execute(stateRef, stateHashVerification);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(
					cache::CatapultCacheDelta& cacheDelta,
					const LocalNodeStateRef& stateRef,
					TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should only be present in the delta
				const auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);

				// Sanity: the view is not modified
				const auto& cacheView = stateRef.Cache.createView();
				EXPECT_EQ(0u, cacheView.sub<cache::AccountStateCache>().size());
			}
		};

		struct ExecuteAndCommitTraits {
			static void Execute(
					NemesisBlockLoader& loader,
					const LocalNodeStateRef& stateRef,
					StateHashVerification stateHashVerification) {
				loader.executeAndCommit(stateRef, stateHashVerification);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(
					cache::CatapultCacheDelta&,
					const LocalNodeStateRef& stateRef,
					TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should be committed to the underlying cache, so check the view
				const auto& cacheView = stateRef.Cache.createView();
				const auto& accountStateCache = cacheView.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);
			}
		};

		struct ExecuteDefaultStateTraits {
			static void Execute(NemesisBlockLoader& loader, const LocalNodeStateRef& stateRef, StateHashVerification) {
				auto pNemesisBlockElement = stateRef.Storage.view().loadBlockElement(Height(1));
				loader.execute(stateRef.Config.BlockChain, *pNemesisBlockElement);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(
					cache::CatapultCacheDelta& cacheDelta,
					const LocalNodeStateRef& stateRef,
					TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should only be present in the delta
				const auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);

				// Sanity: the view is not modified
				const auto& cacheView = stateRef.Cache.createView();
				EXPECT_EQ(0u, cacheView.sub<cache::AccountStateCache>().size());
			}
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExecuteAndCommit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExecuteAndCommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Execute) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExecuteTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExecuteDefaultState) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExecuteDefaultStateTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define TRAITS_BASED_DISABLED_VERIFICATION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExecuteAndCommit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExecuteAndCommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Execute) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExecuteTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region success - with transfers

	namespace {
		template<typename TTraits>
		void AssertCanLoadValidNemesisBlockWithSingleMosaicTransfer(Importance totalChainImportance, Amount totalChainBalance) {
			// Arrange: create a valid nemesis block with a single (mosaic) transaction
			auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(totalChainBalance) } });
			const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

			// Act:
			RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, totalChainImportance, [totalChainBalance, &nemesisBlock](
					const auto& accountStateCache) {
				// Assert:
				EXPECT_EQ(2u, accountStateCache.size());
				const auto& recipient = GetTransactionRecipient(nemesisBlock, 0);
				test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
				test::AssertBalances(accountStateCache, recipient, { MakeHarvestingMosaic(totalChainBalance) });
			});
		}
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_SingleMosaicTransfer) {
		// Assert:
		AssertCanLoadValidNemesisBlockWithSingleMosaicTransfer<TTraits>(Importance(1234), Amount(1234));
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_MultipleMosaicTransfers) {
		// Arrange: create a valid nemesis block with multiple mosaic transactions
		auto nemesisBlockSignerPair = CreateNemesisBlock({
			{ MakeHarvestingMosaic(1234) },
			{ MakeHarvestingMosaic(123), MakeHarvestingMosaic(213) },
			{ MakeHarvestingMosaic(987) }
		});
		const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// Act:
		auto totalChainImportance = Importance(1234 + 123 + 213 + 987);
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, totalChainImportance, [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { MakeHarvestingMosaic(123 + 213) });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 2), { MakeHarvestingMosaic(987) });
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_MultipleHeterogeneousMosaicTransfers) {
		// Arrange: create a valid nemesis block with multiple mosaic transactions
		auto nemesisBlockSignerPair = CreateNemesisBlock({
			{ MakeHarvestingMosaic(1234), { MosaicId(123), Amount(111) }, { MosaicId(444), Amount(222) } },
			{ { MosaicId(123), Amount(987) } },
			{ { MosaicId(333), Amount(213) }, { MosaicId(444), Amount(123) }, { MosaicId(333), Amount(217) } }
		});
		const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// Act:
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, Importance(1234), [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), {
				MakeHarvestingMosaic(1234),
				{ MosaicId(123), Amount(111) },
				{ MosaicId(444), Amount(222) }
			});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { { MosaicId(123), Amount(987) } });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 2), {
				{ MosaicId(333), Amount(213 + 217) },
				{ MosaicId(444), Amount(123) }
			});
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_BalanceMultipleOfImportance) {
		// Assert: (balance == 10000 * importance)
		AssertCanLoadValidNemesisBlockWithSingleMosaicTransfer<TTraits>(Importance(1234), Amount(1234 * 10000));
	}

	// endregion

	// region success - state hash

	TRAITS_BASED_DISABLED_VERIFICATION_TEST(CanLoadNemesisBlockContainingWrongStateHash_VerifiableStateDisabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// - use the wrong state hash
		test::FillWithRandomData(nemesisBlock.StateHash);

		// Act:
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, Importance(1234), StateHashVerification::Disabled, [&nemesisBlock](
				const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_VerifiableStateEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// - create the state (with verifiable state enabled)
		test::TempDirectoryGuard dbDirGuard;
		auto config = CreateDefaultConfiguration(nemesisBlock, Importance(1234));
		auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
		auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
		{
			// - calculate the expected state hash after block execution
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			auto recipient = GetTransactionRecipient(nemesisBlock, 0);
			accountStateCacheDelta.addAccount(nemesisBlock.Signer, Height(1));
			accountStateCacheDelta.addAccount(recipient, Height(1));
			accountStateCacheDelta.find(recipient).get().Balances.credit(Harvesting_Mosaic_Id, Amount(1234));

			nemesisBlock.StateHash = cacheDelta.calculateStateHash(Height(1)).StateHash;

			// Sanity:
			EXPECT_NE(Hash256(), nemesisBlock.StateHash);
		}

		test::LocalNodeTestState state(config, "", std::move(cache));

		SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, NemesisBlockModification::None);

		// - create the publisher, observer and loader
		auto pluginManager = CreatePluginManager();
		auto cacheDelta = state.ref().Cache.createDelta();
		NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

		// Act:
		TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled);

		// Assert:
		TTraits::Assert(cacheDelta, state.ref(), [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
		});
	}

	// endregion

	// region success - receipts block hash

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_VerifiableReceiptsEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// - create the state (with verifiable receipts enabled)
		auto config = CreateDefaultConfiguration(nemesisBlock, Importance(1234));
		config.ShouldEnableVerifiableReceipts = true;
		auto cache = test::CreateEmptyCatapultCache(config);
		{
			// - calculate the expected receipts hash
			model::BlockStatementBuilder blockStatementBuilder;

			// - harvest receipt added by HarvestFeeObserver
			auto receiptType = model::Receipt_Type_Harvest_Fee;
			auto receiptMosaicId = Harvesting_Mosaic_Id;
			blockStatementBuilder.addReceipt(model::BalanceChangeReceipt(receiptType, nemesisBlock.Signer, receiptMosaicId, Amount()));

			// - resolution receipts due to use of CreateResolverContextXor and interaction with MockTransaction
			auto recipient = PublicKeyToAddress(GetTransactionRecipient(nemesisBlock, 0), model::NetworkIdentifier::Mijin_Test);
			blockStatementBuilder.addResolution(test::UnresolveXor(recipient), recipient);
			blockStatementBuilder.addResolution(test::UnresolveXor(receiptMosaicId), receiptMosaicId);

			nemesisBlock.BlockReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());

			// Sanity:
			EXPECT_NE(Hash256(), nemesisBlock.BlockReceiptsHash);
		}

		test::LocalNodeTestState state(config, "", std::move(cache));

		SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, NemesisBlockModification::None);

		// - create the publisher, observer and loader
		auto pluginManager = CreatePluginManager();
		auto cacheDelta = state.ref().Cache.createDelta();
		NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

		// Act:
		TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled);

		// Assert:
		TTraits::Assert(cacheDelta, state.ref(), [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
		});
	}

	// endregion

	// region account states

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_NemesisAccountStateIsCorrectAfterLoading) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// Act:
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, Importance(1234), [&nemesisBlock](const auto& accountStateCache) {
			const auto& publicKey = nemesisBlock.Signer;
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			const auto& accountState = accountStateCache.find(address).get();

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight);
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(1), accountState.PublicKeyHeight);
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_OtherAccountStateIsCorrectAfterLoading) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// Act:
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, Importance(1234), [&nemesisBlock](const auto& accountStateCache) {
			const auto& publicKey = GetTransactionRecipient(nemesisBlock, 0);
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			const auto& accountState = accountStateCache.find(address).get();

			// Assert:
			EXPECT_EQ(Height(1), accountState.AddressHeight);
			EXPECT_EQ(address, accountState.Address);
			EXPECT_EQ(Height(1), accountState.PublicKeyHeight);
			EXPECT_EQ(publicKey, accountState.PublicKey);
		});
	}

	// endregion

	// region failure - balance

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainImportanceTooSmall) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: pass in an incompatible chain importance
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1233));
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(617));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainImportanceTooLarge) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: pass in an incompatible chain importance
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1235));
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(2468));
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(12340));
	}

	// endregion

	// region failure - info

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongNetwork) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong network
		nemesisBlockSignerPair.pBlock->Version ^= 0xFF00;

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongPublicKey) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: use the wrong public key
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234), NemesisBlockModification::Public_Key);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongGenerationHash) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: use the wrong generation hash
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234), NemesisBlockModification::Generation_Hash);
	}

	// endregion

	// region failure - transactions

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingUnknownTransactions) {
		// Arrange: create a valid nemesis block with multiple mosaic transactions but make the second transaction type unknown
		//          so that none of its transfers are processed
		auto nemesisBlockSignerPair = CreateNemesisBlock({
			{ MakeHarvestingMosaic(1234) },
			{ MakeHarvestingMosaic(123), MakeHarvestingMosaic(213) },
			{ MakeHarvestingMosaic(987) }
		});

		// - only MockTransaction::Entity_Type type is registered
		(++nemesisBlockSignerPair.pBlock->Transactions().begin())->Type = static_cast<model::EntityType>(0xFFFF);

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234));
	}

	// endregion

	// region failure - state hash

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongStateHash_VerifiableStateDisabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong state hash
		test::FillWithRandomData(nemesisBlockSignerPair.pBlock->StateHash);

		// Act:
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongStateHash_VerifiableStateEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong state hash
		nemesisBlockSignerPair.pBlock->StateHash = Hash256();

		// Act:
		auto modification = NemesisBlockModification::None;
		auto options = NemesisBlockVerifyOptions::State;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification, options);
	}

	// endregion

	// region failure - receipts hash

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongReceiptsHash_VerifiableReceiptsDisabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong receipts hash
		test::FillWithRandomData(nemesisBlockSignerPair.pBlock->BlockReceiptsHash);

		// Act:
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongReceiptsHash_VerifiableReceiptsEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong receipts hash
		nemesisBlockSignerPair.pBlock->BlockReceiptsHash = Hash256();

		// Act:
		auto modification = NemesisBlockModification::None;
		auto options = NemesisBlockVerifyOptions::Receipts;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification, options);
	}

	// endregion

	// region failure - signature

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongSignature) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act:
		auto modification = NemesisBlockModification::Signature;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification);
	}

	// endregion
}}
