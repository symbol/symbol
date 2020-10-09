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
#include "sdk/src/extensions/BlockExtensions.h"
#include "plugins/coresystem/src/observers/Observers.h"
#include "plugins/coresystem/src/validators/Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/cache/BalanceTransfers.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/plugins/PluginManagerFactory.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NemesisBlockLoaderTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;

		// currency and harvesting mosaic transfers need to be tested for correct behavior
		constexpr auto Currency_Mosaic_Id = MosaicId(3456);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		// region create/set nemesis block

		struct BlockSignerPair {
			std::unique_ptr<crypto::KeyPair> pSigner;
			std::unique_ptr<model::Block> pBlock;
		};

		struct NemesisOptions {
			Importance TotalChainImportance;
			Amount InitialCurrencyAtomicUnits;
			extensions::StateHashVerification StateHashVerification = StateHashVerification::Enabled;
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
				pTransaction->SignerPublicKey = nemesisPublicKey;
				pTransaction->Version = mocks::MockTransaction::Current_Version;
				pTransaction->Network = model::NetworkIdentifier::Private_Test;
				transactions.push_back(std::move(pTransaction));
			}

			blockSignerPair.pBlock = model::CreateBlock(model::PreviousBlockContext(), Network_Identifier, nemesisPublicKey, transactions);
			return blockSignerPair;
		}

		enum class NemesisBlockModification { None, Public_Key, Generation_Hash_Proof, Generation_Hash, Stateless, Stateful };

		void SetNemesisBlock(
				io::BlockStorageCache& storage,
				const BlockSignerPair& blockSignerPair,
				const model::NetworkInfo& network,
				NemesisBlockModification modification) {
			auto pNemesisBlockElement = storage.view().loadBlockElement(Height(1));
			auto storageModifier = storage.modifier();
			storageModifier.dropBlocksAfter(Height(0));

			// copy and prepare the block
			auto pModifiedBlock = test::CopyEntity(*blockSignerPair.pBlock);

			// 1. modify the block signer if requested
			if (NemesisBlockModification::Public_Key == modification)
				test::FillWithRandomData(pModifiedBlock->SignerPublicKey);
			else
				pModifiedBlock->SignerPublicKey = network.NemesisSignerPublicKey;

			// 2. modify the generation hash proof if requested
			if (NemesisBlockModification::Generation_Hash_Proof == modification) {
				test::FillWithRandomData(pModifiedBlock->GenerationHashProof);
			} else {
				auto vrfProof = crypto::GenerateVrfProof(network.GenerationHashSeed, *blockSignerPair.pSigner);
				pModifiedBlock->GenerationHashProof = { vrfProof.Gamma, vrfProof.VerificationHash, vrfProof.Scalar };
			}

			// 3. modify the generation hash if requested
			auto modifiedNemesisBlockElement = test::BlockToBlockElement(*pModifiedBlock);
			if (NemesisBlockModification::Generation_Hash == modification) {
				test::FillWithRandomData(modifiedNemesisBlockElement.GenerationHash);
			} else {
				auto proofHash = crypto::GenerateVrfProofHash(pModifiedBlock->GenerationHashProof.Gamma);
				modifiedNemesisBlockElement.GenerationHash = proofHash.copyTo<GenerationHash>();
			}

			// 4. modify the block to fail stateless validation
			if (NemesisBlockModification::Stateless == modification)
				++pModifiedBlock->Version;

			// 5. modify the block to fail stateful validation
			if (NemesisBlockModification::Stateful == modification)
				test::FillWithRandomData(pModifiedBlock->BeneficiaryAddress);

			storageModifier.saveBlock(modifiedNemesisBlockElement);
			storageModifier.commit();
		}

		// endregion

		// region utils

		model::Mosaic MakeCurrencyMosaic(Amount::ValueType amount) {
			return { Currency_Mosaic_Id, Amount(amount) };
		}

		model::Mosaic MakeHarvestingMosaic(Amount amount) {
			return { Harvesting_Mosaic_Id, amount };
		}

		model::Mosaic MakeHarvestingMosaic(Amount::ValueType amount) {
			return MakeHarvestingMosaic(Amount(amount));
		}

		model::BlockChainConfiguration CreateDefaultConfiguration(const model::Block& nemesisBlock, const NemesisOptions& nemesisOptions) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.Network.NemesisSignerPublicKey = nemesisBlock.SignerPublicKey;
			test::FillWithRandomData(config.Network.GenerationHashSeed);
			config.CurrencyMosaicId = Currency_Mosaic_Id;
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.InitialCurrencyAtomicUnits = nemesisOptions.InitialCurrencyAtomicUnits;
			config.MaxMosaicAtomicUnits = Amount(15'000'000);
			config.TotalChainImportance = nemesisOptions.TotalChainImportance;
			return config;
		}

		plugins::PluginManager CreatePluginManager() {
			// enable Publish_Transfers (MockTransaction Publish XORs recipient address, so XOR address resolver is required
			// for proper roundtripping or else test will fail)
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			auto manager = test::CreatePluginManager(config);
			manager.addTransactionSupport(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));
			manager.addTransactionSupport(mocks::CreateMockTransactionPlugin(
					static_cast<model::EntityType>(0xFFFE),
					mocks::PluginOptionFlags::Not_Top_Level));

			manager.addMosaicResolver([](const auto&, const auto& unresolved, auto& resolved) {
				resolved = test::CreateResolverContextXor().resolve(unresolved);
				return true;
			});
			manager.addAddressResolver([](const auto&, const auto& unresolved, auto& resolved) {
				resolved = test::CreateResolverContextXor().resolve(unresolved);
				return true;
			});

			manager.addStatelessValidatorHook([](auto& builder) {
				builder.add(validators::CreateEntityVersionValidator());
			});

			manager.addStatefulValidatorHook([](auto& builder) {
				builder.add(validators::CreateAddressValidator());
			});
			return manager;
		}

		std::unique_ptr<const observers::NotificationObserver> CreateObserverWithHarvestNetworkFees(
				uint8_t harvestNetworkPercentage,
				const Address& harvestNetworkFeeSinkAddress) {
			// use real coresystem observers to create accounts, update balances and add harvest receipt
			observers::DemuxObserverBuilder builder;
			builder
				.add(observers::CreateAccountAddressObserver())
				.add(observers::CreateAccountPublicKeyObserver())
				.add(observers::CreateBalanceTransferObserver())
				.add(observers::CreateHarvestFeeObserver(
						{ Harvesting_Mosaic_Id, 20, harvestNetworkPercentage, harvestNetworkFeeSinkAddress },
						model::InflationCalculator()));
			return builder.build();
		}

		std::unique_ptr<const observers::NotificationObserver> CreateObserver() {
			return CreateObserverWithHarvestNetworkFees(0, Address());
		}

		const Key& GetTransactionRecipient(const model::Block& block, size_t index) {
			auto transactions = block.Transactions();
			auto iter = transactions.cbegin();
			for (auto i = 0u; i < index; ++i, ++iter);
			return static_cast<const mocks::MockTransaction&>(*iter).RecipientPublicKey;
		}

		// endregion

		// region generic tests

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const BlockSignerPair& nemesisBlockSignerPair,
				const NemesisOptions& nemesisOptions,
				TAssertAccountStateCache assertAccountStateCache) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(*nemesisBlockSignerPair.pBlock, nemesisOptions);
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, NemesisBlockModification::None);

			// - create the publisher, observer and loader
			auto pluginManager = CreatePluginManager();
			{
				auto cacheDelta = state.ref().Cache.createDelta();
				NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

				// Act:
				TTraits::Execute(loader, state.ref(), nemesisOptions.StateHashVerification);

				// Assert:
				TTraits::Assert(cacheDelta, assertAccountStateCache);
			}

			auto cacheView = state.ref().Cache.createView();
			TTraits::Assert(cacheView, assertAccountStateCache);
		}

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const BlockSignerPair& nemesisBlockSignerPair,
				Importance totalChainImportance,
				TAssertAccountStateCache assertAccountStateCache) {
			NemesisOptions nemesisOptions{ totalChainImportance, Amount() };
			RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, nemesisOptions, assertAccountStateCache);
		}

		enum class NemesisBlockVerifyOptions { None, State, Receipts };

		template<typename TTraits, typename TException = catapult_invalid_argument>
		void AssertLoadNemesisBlockFailure(
				const BlockSignerPair& nemesisBlockSignerPair,
				const NemesisOptions& nemesisOptions,
				NemesisBlockModification modification = NemesisBlockModification::None,
				NemesisBlockVerifyOptions verifyOptions = NemesisBlockVerifyOptions::None) {
			// Arrange:
			bool enableVerifiableState = NemesisBlockVerifyOptions::State == verifyOptions;

			// - create the state
			auto config = CreateDefaultConfiguration(*nemesisBlockSignerPair.pBlock, nemesisOptions);
			config.EnableVerifiableReceipts = NemesisBlockVerifyOptions::Receipts == verifyOptions;

			auto cacheConfig = cache::CacheConfiguration();
			test::TempDirectoryGuard dbDirGuard;
			if (enableVerifiableState)
				cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

			auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
			test::LocalNodeTestState state(config, "", std::move(cache));
			SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, modification);

			{
				// Sanity: state hash calculation is configured properly on the cache
				auto cacheDelta = state.ref().Cache.createDelta();
				cacheDelta.sub<cache::AccountStateCache>().addAccount(test::GenerateRandomByteArray<Address>(), Height(1));

				auto cacheStateHash = cacheDelta.calculateStateHash(Height(1)).StateHash;
				if (enableVerifiableState)
					EXPECT_NE(Hash256(), cacheStateHash);
				else
					EXPECT_EQ(Hash256(), cacheStateHash);
			}

			// Arrange:  create the publisher, observer and loader
			auto pluginManager = CreatePluginManager();
			auto cacheDelta = state.ref().Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, pluginManager, CreateObserver());

			// Act + Assert:
			EXPECT_THROW(TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled), TException)
					<< "total chain importance " << nemesisOptions.TotalChainImportance
					<< ", initial currency atomic units " << nemesisOptions.InitialCurrencyAtomicUnits;
		}

		template<typename TTraits, typename TException = catapult_invalid_argument>
		void AssertLoadNemesisBlockFailure(
				const BlockSignerPair& nemesisBlockSignerPair,
				Importance totalChainImportance,
				NemesisBlockModification modification = NemesisBlockModification::None,
				NemesisBlockVerifyOptions verifyOptions = NemesisBlockVerifyOptions::None) {
			NemesisOptions nemesisOptions{ totalChainImportance, Amount() };
			AssertLoadNemesisBlockFailure<TTraits, TException>(nemesisBlockSignerPair, nemesisOptions, modification, verifyOptions);
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
			static void Assert(cache::CatapultCacheDelta& cacheDelta, TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should only be present in the delta
				const auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(cache::CatapultCacheView& cacheView, TAssertAccountStateCache) {
				// Sanity: the view is not modified
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
			static void Assert(cache::CatapultCacheDelta& cacheDelta, TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should be present in the delta
				const auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(cache::CatapultCacheView& cacheView, TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should be committed to the underlying cache, so check the view
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
			static void Assert(cache::CatapultCacheDelta& cacheDelta, TAssertAccountStateCache assertAccountStateCache) {
				// Assert: changes should only be present in the delta
				const auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				assertAccountStateCache(accountStateCache);
			}

			template<typename TAssertAccountStateCache>
			static void Assert(cache::CatapultCacheView& cacheView, TAssertAccountStateCache) {
				// Sanity: the view is not modified
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
				test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
				test::AssertBalances(accountStateCache, recipient, { MakeHarvestingMosaic(totalChainBalance) });
			});
		}
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_SingleMosaicTransfer) {
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
			test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { MakeHarvestingMosaic(123 + 213) });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 2), { MakeHarvestingMosaic(987) });
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_MultipleHeterogeneousMosaicTransfersIncludingCurrencyMosaic) {
		// Arrange: create a valid nemesis block with multiple mosaic transactions
		auto nemesisBlockSignerPair = CreateNemesisBlock({
			{ MakeHarvestingMosaic(1234), { MosaicId(123), Amount(111) }, { MosaicId(444), Amount(222) } },
			{ MakeCurrencyMosaic(987) },
			{ { MosaicId(333), Amount(213) }, { MosaicId(444), Amount(123) }, { MosaicId(333), Amount(217) } }
		});
		const auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// Act:
		NemesisOptions nemesisOptions{ Importance(1234), Amount(987) };
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, nemesisOptions, [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), {
				MakeHarvestingMosaic(1234),
				{ MosaicId(123), Amount(111) },
				{ MosaicId(444), Amount(222) }
			});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { MakeCurrencyMosaic(987) });
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
		NemesisOptions nemesisOptions{ Importance(1234), Amount(), StateHashVerification::Disabled };
		RunLoadNemesisBlockTest<TTraits>(nemesisBlockSignerPair, nemesisOptions, [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { MakeHarvestingMosaic(1234) });
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_VerifiableStateEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
		auto& nemesisBlock = *nemesisBlockSignerPair.pBlock;

		// - create the state (with verifiable state enabled)
		test::TempDirectoryGuard dbDirGuard;
		NemesisOptions nemesisOptions{ Importance(1234), Amount() };
		auto config = CreateDefaultConfiguration(nemesisBlock, nemesisOptions);
		auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
		auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
		{
			// - calculate the expected state hash after block execution
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			auto recipient = GetTransactionRecipient(nemesisBlock, 0);
			accountStateCacheDelta.addAccount(nemesisBlock.SignerPublicKey, Height(1));
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
		TTraits::Assert(cacheDelta, [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
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
		NemesisOptions nemesisOptions{ Importance(1234), Amount() };
		auto config = CreateDefaultConfiguration(nemesisBlock, nemesisOptions);
		config.EnableVerifiableReceipts = true;
		auto cache = test::CreateEmptyCatapultCache(config);
		{
			// - calculate the expected receipts hash
			model::BlockStatementBuilder blockStatementBuilder;

			// - harvest receipt added by HarvestFeeObserver
			auto receiptType = model::Receipt_Type_Harvest_Fee;
			auto receiptMosaicId = Harvesting_Mosaic_Id;
			blockStatementBuilder.addReceipt(model::BalanceChangeReceipt(
					receiptType,
					model::GetSignerAddress(nemesisBlock),
					receiptMosaicId,
					Amount()));

			// - resolution receipts due to use of CreateResolverContextXor and interaction with MockTransaction
			auto recipient = model::PublicKeyToAddress(GetTransactionRecipient(nemesisBlock, 0), model::NetworkIdentifier::Private_Test);
			blockStatementBuilder.addResolution(test::UnresolveXor(recipient), recipient);
			blockStatementBuilder.addResolution(test::UnresolveXor(receiptMosaicId), receiptMosaicId);

			nemesisBlock.ReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());

			// Sanity:
			EXPECT_NE(Hash256(), nemesisBlock.ReceiptsHash);
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
		TTraits::Assert(cacheDelta, [&nemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.SignerPublicKey, {});
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
			const auto& publicKey = nemesisBlock.SignerPublicKey;
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

	namespace {
		template<typename TTraits, typename TAssertAccountStateCache>
		void RunNemesisBlockSpecialSinkAccountTest(
				uint8_t harvestNetworkPercentage,
				const Address& harvestNetworkFeeSinkAddress,
				TAssertAccountStateCache assertAccountStateCache) {
			// Arrange: create a valid nemesis block with a single (mosaic) transaction
			auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });
			auto nemesisOptions = NemesisOptions{ Importance(1234), Amount() };

			// - create the state
			auto config = CreateDefaultConfiguration(*nemesisBlockSignerPair.pBlock, nemesisOptions);
			config.HarvestNetworkPercentage = harvestNetworkPercentage;
			config.HarvestNetworkFeeSinkAddress = harvestNetworkFeeSinkAddress;
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlockSignerPair, config.Network, NemesisBlockModification::None);

			// - create the publisher, observer and loader
			auto pluginManager = CreatePluginManager();
			{
				auto cacheDelta = state.ref().Cache.createDelta();
				NemesisBlockLoader loader(
						cacheDelta,
						pluginManager,
						CreateObserverWithHarvestNetworkFees(harvestNetworkPercentage, harvestNetworkFeeSinkAddress));

				// Act:
				TTraits::Execute(loader, state.ref(), nemesisOptions.StateHashVerification);

				// Assert:
				TTraits::Assert(cacheDelta, assertAccountStateCache);
			}

			auto cacheView = state.ref().Cache.createView();
			TTraits::Assert(cacheView, assertAccountStateCache);
		}
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_SpecialSinkAccountIsCreatedWhenEnabled) {
		// Arrange:
		auto sinkAddress = test::GenerateRandomAddress(Network_Identifier);

		// Act:
		RunNemesisBlockSpecialSinkAccountTest<TTraits>(10, sinkAddress, [&sinkAddress](const auto& accountStateCache) {
			auto accountStateIter = accountStateCache.find(sinkAddress);

			// Assert:
			EXPECT_TRUE(!!accountStateIter.tryGet());
			EXPECT_EQ(Height(1), accountStateIter.get().AddressHeight);
			EXPECT_EQ(sinkAddress, accountStateIter.get().Address);
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_SpecialSinkAccountIsNotCreatedWhenDisabled) {
		// Arrange:
		auto sinkAddress = test::GenerateRandomAddress(Network_Identifier);

		// Act:
		RunNemesisBlockSpecialSinkAccountTest<TTraits>(0, sinkAddress, [&sinkAddress](const auto& accountStateCache) {
			auto accountStateIter = accountStateCache.find(sinkAddress);

			// Assert:
			EXPECT_FALSE(!!accountStateIter.tryGet());
		});
	}

	// endregion

	// region failure - balance

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainCurrencyTooSmall) {
		// Arrange: create a valid nemesis block
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234), MakeCurrencyMosaic(1234) } });

		// Act: pass in an incompatible total chain currency
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(1233) });
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(617) });
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainCurrencyTooLarge) {
		// Arrange: create a valid nemesis block
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234), MakeCurrencyMosaic(1234) } });

		// Act: pass in an incompatible total chain currency
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(1235) });
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(2468) });
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(12340) });
	}

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

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalMosaicSupplyExceedingMaxAtomicUnits) {
		// Arrange: create an invalid nemesis block, max atomic units is 15'000'000
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234), { MosaicId(345), Amount(15'000'001) } } });

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, { Importance(1234), Amount(0) });
	}

	// endregion

	// region failure - info

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongNetwork) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong network
		nemesisBlockSignerPair.pBlock->Network = model::NetworkIdentifier::Zero;

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongPublicKey) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: use the wrong public key
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234), NemesisBlockModification::Public_Key);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongGenerationHashProof) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: use the wrong generation hash proof
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234), NemesisBlockModification::Generation_Hash_Proof);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongGenerationHash) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act: use the wrong generation hash
		AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234), NemesisBlockModification::Generation_Hash);
	}

	// endregion

	// region failure - transactions

	namespace {
		template<typename TTraits>
		void AssertInvalidTransactionType(model::EntityType invalidTransactionType) {
			// Arrange: create a valid nemesis block with multiple mosaic transactions but make the second transaction type invalid
			//          so that none of its transfers are processed
			auto nemesisBlockSignerPair = CreateNemesisBlock({
				{ MakeHarvestingMosaic(1234) },
				{ MakeHarvestingMosaic(123), MakeHarvestingMosaic(213) },
				{ MakeHarvestingMosaic(987) }
			});

			(++nemesisBlockSignerPair.pBlock->Transactions().begin())->Type = invalidTransactionType;

			// Act:
			AssertLoadNemesisBlockFailure<TTraits>(nemesisBlockSignerPair, Importance(1234));
		}
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingUnknownTransactions) {
		// Assert: 0xFFFF is not registered
		AssertInvalidTransactionType<TTraits>(static_cast<model::EntityType>(0xFFFF));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingNonTopLevelTransactions) {
		// Assert: 0xFFFE does not have top-level support
		AssertInvalidTransactionType<TTraits>(static_cast<model::EntityType>(0xFFFE));
	}

	// endregion

	// region failure - fee multiplier

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithNonzeroFeeMultiplier) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use non-zero block fee multiplier
		nemesisBlockSignerPair.pBlock->FeeMultiplier = BlockFeeMultiplier(12);

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
		test::FillWithRandomData(nemesisBlockSignerPair.pBlock->ReceiptsHash);

		// Act:
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongReceiptsHash_VerifiableReceiptsEnabled) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// - use the wrong receipts hash
		nemesisBlockSignerPair.pBlock->ReceiptsHash = Hash256();

		// Act:
		auto modification = NemesisBlockModification::None;
		auto options = NemesisBlockVerifyOptions::Receipts;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification, options);
	}

	// endregion

	// region failure - validation

	TRAITS_BASED_TEST(CannotLoadNemesisBlockThatFailsStatelessValidation) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act:
		auto modification = NemesisBlockModification::Stateless;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockThatFailsStatefulValidation) {
		// Arrange: create a valid nemesis block with a single (mosaic) transaction
		auto nemesisBlockSignerPair = CreateNemesisBlock({ { MakeHarvestingMosaic(1234) } });

		// Act:
		auto modification = NemesisBlockModification::Stateful;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(nemesisBlockSignerPair, Importance(1234), modification);
	}

	// endregion
}}
