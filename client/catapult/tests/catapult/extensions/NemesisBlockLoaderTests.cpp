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
#include "catapult/model/BlockUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BalanceTransfers.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NemesisBlockLoaderTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		std::unique_ptr<model::Block> CreateNemesisBlock(const std::vector<test::BalanceTransfers>& transferGroups) {
			auto nemesisPublicKey = test::GenerateRandomData<Key_Size>();
			model::Transactions transactions;
			for (const auto& transfers : transferGroups) {
				auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), transfers);
				pTransaction->Signer = nemesisPublicKey;
				transactions.push_back(std::move(pTransaction));
			}

			return CreateBlock(model::PreviousBlockContext(), Network_Identifier, nemesisPublicKey, transactions);
		}

		enum class NemesisBlockModification { None, Public_Key, Generation_Hash };

		void SetNemesisBlock(
				io::BlockStorageCache& storage,
				const model::Block& block,
				const model::NetworkInfo& network,
				NemesisBlockModification modification) {
			// note that this trick only works for MockMemoryBlockStorage, which allows the nemesis block to be dropped
			auto pNemesisBlockElement = storage.view().loadBlockElement(Height(1));
			auto storageModifier = storage.modifier();
			storageModifier.dropBlocksAfter(Height(0));

			// modify the block signer if requested
			auto pModifiedBlock = test::CopyBlock(block);
			if (NemesisBlockModification::Public_Key == modification)
				test::FillWithRandomData(pModifiedBlock->Signer);
			else
				pModifiedBlock->Signer = network.PublicKey;

			// modify the generation hash if requested
			auto modifiedNemesisBlockElement = test::BlockToBlockElement(*pModifiedBlock);
			if (NemesisBlockModification::Generation_Hash == modification)
				test::FillWithRandomData(modifiedNemesisBlockElement.GenerationHash);
			else
				modifiedNemesisBlockElement.GenerationHash = network.GenerationHash;

			storageModifier.saveBlock(modifiedNemesisBlockElement);
		}

		model::BlockChainConfiguration CreateDefaultConfiguration(const model::Block& nemesisBlock, Amount totalChainBalance) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.Network.PublicKey = nemesisBlock.Signer;
			test::FillWithRandomData(config.Network.GenerationHash);
			config.TotalChainBalance = totalChainBalance;
			return config;
		}

		model::TransactionRegistry CreateTransactionRegistry() {
			model::TransactionRegistry registry;
			registry.registerPlugin(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));
			return registry;
		}

		std::unique_ptr<const observers::EntityObserver> CreateObserver(const model::TransactionRegistry& transactionRegistry) {
			// use real coresystem observers to create accounts and update balances
			observers::DemuxObserverBuilder builder;
			builder
				.add(observers::CreateAccountAddressObserver())
				.add(observers::CreateAccountPublicKeyObserver())
				.add(observers::CreateBalanceTransferObserver());
			auto pPublisher = model::CreateNotificationPublisher(transactionRegistry, model::PublisherContext());
			return std::make_unique<observers::NotificationObserverAdapter>(builder.build(), std::move(pPublisher));
		}

		const Key& GetTransactionRecipient(const model::Block& block, size_t index) {
			auto transactions = block.Transactions();
			auto iter = transactions.cbegin();
			for (auto i = 0u; i < index; ++i, ++iter);
			return static_cast<const mocks::MockTransaction&>(*iter).Recipient;
		}

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const model::Block& nemesisBlock,
				Amount totalChainBalance,
				StateHashVerification stateHashVerification,
				TAssertAccountStateCache assertAccountStateCache) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(nemesisBlock, totalChainBalance);
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlock, config.Network, NemesisBlockModification::None);

			// - create the publisher, observer and loader
			auto transactionRegistry = CreateTransactionRegistry();
			auto pPublisher = model::CreateNotificationPublisher(transactionRegistry, model::PublisherContext());
			auto pObserver = CreateObserver(transactionRegistry);
			auto cacheDelta = state.ref().Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, transactionRegistry, *pPublisher, *pObserver);

			// Act:
			TTraits::Execute(loader, state.ref(), stateHashVerification);

			// Assert:
			TTraits::Assert(cacheDelta, state.ref(), assertAccountStateCache);
		}

		template<typename TTraits, typename TAssertAccountStateCache>
		void RunLoadNemesisBlockTest(
				const model::Block& nemesisBlock,
				Amount totalChainBalance,
				TAssertAccountStateCache assertAccountStateCache) {
			RunLoadNemesisBlockTest<TTraits>(nemesisBlock, totalChainBalance, StateHashVerification::Enabled, assertAccountStateCache);
		}

		template<typename TTraits, typename TException = catapult_invalid_argument>
		void AssertLoadNemesisBlockFailure(
				const model::Block& nemesisBlock,
				Amount totalChainBalance,
				NemesisBlockModification modification = NemesisBlockModification::None,
				bool enableVerifiableState = false) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(nemesisBlock, totalChainBalance);
			auto cacheConfig = cache::CacheConfiguration();
			test::TempDirectoryGuard dbDirGuard("testdb");
			if (enableVerifiableState)
				cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

			auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
			test::LocalNodeTestState state(config, "", std::move(cache));
			SetNemesisBlock(state.ref().Storage, nemesisBlock, config.Network, modification);

			// - create the publisher, observer and loader
			auto transactionRegistry = CreateTransactionRegistry();
			auto pPublisher = model::CreateNotificationPublisher(transactionRegistry, model::PublisherContext());
			auto pObserver = CreateObserver(transactionRegistry);
			auto cacheDelta = state.ref().Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, transactionRegistry, *pPublisher, *pObserver);

			// Act + Assert:
			EXPECT_THROW(TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled), TException);

			// Sanity:
			auto cacheStateHash = state.cref().Cache.createView().calculateStateHash().StateHash;
			if (enableVerifiableState)
				EXPECT_NE(Hash256(), cacheStateHash);
			else
				EXPECT_EQ(Hash256(), cacheStateHash);
		}

		struct ExecuteTraits {
			static void Execute(
					const NemesisBlockLoader& loader,
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
					const NemesisBlockLoader& loader,
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
			static void Execute(const NemesisBlockLoader& loader, const LocalNodeStateRef& stateRef, StateHashVerification) {
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

	// region success

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_SingleXemTransfer) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest<TTraits>(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
		});
	}

	TRAITS_BASED_DISABLED_VERIFICATION_TEST(CanLoadNemesisBlockContainingWrongStateHash_VerifiableStateDisabled) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - use the wrong state hash
		pNemesisBlock->StateHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		RunLoadNemesisBlockTest<TTraits>(
				*pNemesisBlock,
				Amount(1234),
				StateHashVerification::Disabled,
				[&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
					// Assert:
					EXPECT_EQ(2u, accountStateCache.size());
					test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
					test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
				});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_MultipleXemTransfers) {
		// Arrange: create a valid nemesis block with multiple xem transactions
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) } },
			{ { Xem_Id, Amount(123) }, { Xem_Id, Amount(213) } },
			{ { Xem_Id, Amount(987) } }
		});

		// Act:
		auto totalChainBalance = Amount(1234 + 123 + 213 + 987);
		RunLoadNemesisBlockTest<TTraits>(*pNemesisBlock, totalChainBalance, [&nemesisBlock = *pNemesisBlock](
				const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { { Xem_Id, Amount(123 + 213) } });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 2), { { Xem_Id, Amount(987) } });
		});
	}

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_MultipleMosaicTransfers) {
		// Arrange: create a valid nemesis block with multiple xem and mosaic transactions
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) }, { MosaicId(123), Amount(111) }, { MosaicId(444), Amount(222) } },
			{ { MosaicId(123), Amount(987) } },
			{ { MosaicId(333), Amount(213) }, { MosaicId(444), Amount(123) }, { MosaicId(333), Amount(217) } }
		});

		// Act:
		RunLoadNemesisBlockTest<TTraits>(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), {
				{ Xem_Id, Amount(1234) },
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

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_VerifiableStateEnabled) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - create the state (with verifiable state enabled)
		test::TempDirectoryGuard dbDirGuard("testdb");
		auto config = CreateDefaultConfiguration(*pNemesisBlock, Amount(1234));
		auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
		auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
		{
			// - calculate the expected state hash after block execution
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			auto recipient = GetTransactionRecipient(*pNemesisBlock, 0);
			accountStateCacheDelta.addAccount(pNemesisBlock->Signer, Height(1));
			accountStateCacheDelta.addAccount(recipient, Height(1));
			accountStateCacheDelta.find(recipient).get().Balances.credit(Xem_Id, Amount(1234));

			pNemesisBlock->StateHash = cacheDelta.calculateStateHash(Height(1)).StateHash;

			// Sanity:
			EXPECT_NE(Hash256(), pNemesisBlock->StateHash);
		}

		test::LocalNodeTestState state(config, "", std::move(cache));

		SetNemesisBlock(state.ref().Storage, *pNemesisBlock, config.Network, NemesisBlockModification::None);

		// - create the publisher, observer and loader
		auto transactionRegistry = CreateTransactionRegistry();
		auto pPublisher = model::CreateNotificationPublisher(transactionRegistry, model::PublisherContext());
		auto pObserver = CreateObserver(transactionRegistry);
		auto cacheDelta = state.ref().Cache.createDelta();
		NemesisBlockLoader loader(cacheDelta, transactionRegistry, *pPublisher, *pObserver);

		// Act:
		TTraits::Execute(loader, state.ref(), StateHashVerification::Enabled);

		// Assert:
		TTraits::Assert(cacheDelta, state.ref(), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
		});
	}

	// endregion

	// region account states

	TRAITS_BASED_TEST(CanLoadValidNemesisBlock_NemesisAccountStateIsCorrectAfterLoading) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest<TTraits>(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
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
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest<TTraits>(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
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

	// region failure

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainBalanceTooSmall) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: pass in the wrong chain balance
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1233));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithTotalChainBalanceTooLarge) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: pass in the wrong chain balance
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1235));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongNetwork) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - use the wrong network
		pNemesisBlock->Version ^= 0xFF00;

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongPublicKey) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: use the wrong public key
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1234), NemesisBlockModification::Public_Key);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockWithWrongGenerationHash) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: use the wrong generation hash
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1234), NemesisBlockModification::Generation_Hash);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingUnknownTransactions) {
		// Arrange: create a valid nemesis block with multiple xem transactions but make the second transaction type unknown
		//          so that none of its transfers are processed
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) } },
			{ { Xem_Id, Amount(123) }, { Xem_Id, Amount(213) } },
			{ { Xem_Id, Amount(987) } }
		});

		// - only MockTransaction::Entity_Type type is registered
		(++pNemesisBlock->Transactions().begin())->Type = static_cast<model::EntityType>(0xFFFF);

		// Act:
		AssertLoadNemesisBlockFailure<TTraits>(*pNemesisBlock, Amount(1234));
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongStateHash_VerifiableStateDisabled) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - use the wrong state hash
		pNemesisBlock->StateHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto modification = NemesisBlockModification::None;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(*pNemesisBlock, Amount(1234), modification, false);
	}

	TRAITS_BASED_TEST(CannotLoadNemesisBlockContainingWrongStateHash_VerifiableStateEnabled) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - use the wrong state hash
		pNemesisBlock->StateHash = Hash256();

		// Act:
		auto modification = NemesisBlockModification::None;
		AssertLoadNemesisBlockFailure<TTraits, catapult_runtime_error>(*pNemesisBlock, Amount(1234), modification, true);
	}

	// endregion
}}
