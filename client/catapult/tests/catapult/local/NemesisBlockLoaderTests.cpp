#include "catapult/local/NemesisBlockLoader.h"
#include "plugins/coresystem/src/observers/Observers.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/local/NotificationObserverAdapter.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/BalanceTransfers.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"

namespace catapult { namespace local {

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

		enum class NemesisBlockModification {
			None,
			Public_Key,
			Generation_Hash
		};

		void SetNemesisBlock(
				io::BlockStorageCache& storage,
				const model::Block& block,
				const model::NetworkInfo& network,
				NemesisBlockModification modification) {
			// note that this trick only works for MemoryBasedStorage, which allows the nemesis block to be dropped
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
				.add(observers::CreateBalanceObserver());
			return std::make_unique<NotificationObserverAdapter>(transactionRegistry, builder.build());
		}

		const Key& GetTransactionRecipient(const model::Block& block, size_t index) {
			auto iter = block.Transactions().cbegin();
			for (auto i = 0u; i < index; ++i, ++iter);
			return static_cast<const mocks::MockTransaction&>(*iter).Recipient;
		}

		template<typename TAssertAccountState>
		void RunLoadNemesisBlockTest(const model::Block& nemesisBlock, Amount totalChainBalance, TAssertAccountState assertAccountState) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(nemesisBlock, totalChainBalance);
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlock, config.Network, NemesisBlockModification::None);

			// - create the observer
			auto transactionRegistry = CreateTransactionRegistry();
			auto pObserver = CreateObserver(transactionRegistry);

			// Act:
			LoadNemesisBlock(transactionRegistry, *pObserver, state.ref());

			// Assert:
			const auto& cacheView = state.cref().Cache.createView();
			const auto& accountStateCache = cacheView.sub<cache::AccountStateCache>();
			assertAccountState(accountStateCache);
		}

		void AssertLoadNemesisBlockFailure(
				const model::Block& nemesisBlock,
				Amount totalChainBalance,
				NemesisBlockModification modification = NemesisBlockModification::None) {
			// Arrange: create the state
			auto config = CreateDefaultConfiguration(nemesisBlock, totalChainBalance);
			test::LocalNodeTestState state(config);
			SetNemesisBlock(state.ref().Storage, nemesisBlock, config.Network, modification);

			// - create the observer
			auto transactionRegistry = CreateTransactionRegistry();
			auto pObserver = CreateObserver(transactionRegistry);

			// Act:
			EXPECT_THROW(
					LoadNemesisBlock(transactionRegistry, *pObserver, state.ref()),
					catapult_invalid_argument);
		}
	}

	// region success

	TEST(TEST_CLASS, CanLoadValidNemesisBlock_SingleXemTransfer) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(2u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
		});
	}

	TEST(TEST_CLASS, CanLoadValidNemesisBlock_MultipleXemTransfers) {
		// Arrange: create a valid nemesis block with multiple xem transactions
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) } },
			{ { Xem_Id, Amount(123) }, { Xem_Id, Amount(213) } },
			{ { Xem_Id, Amount(987) } }
		});

		// Act:
		auto totalChainBalance = Amount(1234 + 123 + 213 + 987);
		RunLoadNemesisBlockTest(*pNemesisBlock, totalChainBalance, [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			// Assert:
			EXPECT_EQ(4u, accountStateCache.size());
			test::AssertBalances(accountStateCache, nemesisBlock.Signer, {});
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 0), { { Xem_Id, Amount(1234) } });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 1), { { Xem_Id, Amount(123 + 213) } });
			test::AssertBalances(accountStateCache, GetTransactionRecipient(nemesisBlock, 2), { { Xem_Id, Amount(987) } });
		});
	}

	TEST(TEST_CLASS, CanLoadValidNemesisBlock_MultipleMosaicTransfers) {
		// Arrange: create a valid nemesis block with multiple xem and mosaic transactions
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) }, { MosaicId(123), Amount(111) }, { MosaicId(444), Amount(222) } },
			{ { MosaicId(123), Amount(987) } },
			{ { MosaicId(333), Amount(213) }, { MosaicId(444), Amount(123) }, { MosaicId(333), Amount(217) } }
		});

		// Act:
		RunLoadNemesisBlockTest(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
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

	// endregion

	// region account states

	TEST(TEST_CLASS, CanLoadValidNemesisBlock_NemesisAccountStateIsCorrectAfterLoading) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			const auto& publicKey = nemesisBlock.Signer;
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			auto pAccountState = accountStateCache.findAccount(address);

			// Assert:
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(Height(1), pAccountState->AddressHeight);
			EXPECT_EQ(address, pAccountState->Address);
			EXPECT_EQ(Height(1), pAccountState->PublicKeyHeight);
			EXPECT_EQ(publicKey, pAccountState->PublicKey);
		});
	}

	TEST(TEST_CLASS, CanLoadValidNemesisBlock_OtherAccountStateIsCorrectAfterLoading) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act:
		RunLoadNemesisBlockTest(*pNemesisBlock, Amount(1234), [&nemesisBlock = *pNemesisBlock](const auto& accountStateCache) {
			const auto& publicKey = GetTransactionRecipient(nemesisBlock, 0);
			auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);
			auto pAccountState = accountStateCache.findAccount(address);

			// Assert:
			ASSERT_TRUE(!!pAccountState);
			EXPECT_EQ(Height(1), pAccountState->AddressHeight);
			EXPECT_EQ(address, pAccountState->Address);
			EXPECT_EQ(Height(1), pAccountState->PublicKeyHeight);
			EXPECT_EQ(publicKey, pAccountState->PublicKey);
		});
	}

	// endregion

	// region failure

	TEST(TEST_CLASS, CannotLoadNemesisBlockWithTotalChainBalanceTooSmall) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: pass in the wrong chain balance
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1233));
	}

	TEST(TEST_CLASS, CannotLoadNemesisBlockWithTotalChainBalanceTooLarge) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: pass in the wrong chain balance
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1235));
	}

	TEST(TEST_CLASS, CannotLoadNemesisBlockWithWrongNetwork) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// - use the wrong network
		pNemesisBlock->Version ^= 0xFF00;

		// Act:
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1234));
	}

	TEST(TEST_CLASS, CannotLoadNemesisBlockWithWrongPublicKey) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: use the wrong public key
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1234), NemesisBlockModification::Public_Key);
	}

	TEST(TEST_CLASS, CannotLoadNemesisBlockWithWrongGenerationHash) {
		// Arrange: create a valid nemesis block with a single (xem) transaction
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });

		// Act: use the wrong generation hash
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1234), NemesisBlockModification::Generation_Hash);
	}

	TEST(TEST_CLASS, CannotLoadNemesisBlockContainingUnknownTransactions) {
		// Arrange: create a valid nemesis block with multiple xem transactions but make the second transaction type unknown
		//          so that none of its transfers are processed
		auto pNemesisBlock = CreateNemesisBlock({
			{ { Xem_Id, Amount(1234) } },
			{ { Xem_Id, Amount(123) }, { Xem_Id, Amount(213) } },
			{ { Xem_Id, Amount(987) } }
		});

		// - only MockTransaction::Entity_Type type is registered, so EntityType::Transfer is unknown
		(++pNemesisBlock->Transactions().begin())->Type = model::EntityType::Transfer;

		// Act:
		AssertLoadNemesisBlockFailure(*pNemesisBlock, Amount(1234));
	}

	// endregion

	// region PreCommit

	TEST(TEST_CLASS, LoadNemesisBlockCallsPreCommitFunction) {
		// Arrange:
		auto numPreCommitCalls = 0u;
		auto pNemesisBlock = CreateNemesisBlock({ { { Xem_Id, Amount(1234) } } });
		auto config = CreateDefaultConfiguration(*pNemesisBlock, Amount(1234));
		test::LocalNodeTestState state(config);
		SetNemesisBlock(state.ref().Storage, *pNemesisBlock, config.Network, NemesisBlockModification::None);

		// - create the observer
		auto transactionRegistry = CreateTransactionRegistry();
		auto pObserver = CreateObserver(transactionRegistry);

		// Act:
		LoadNemesisBlock(transactionRegistry, *pObserver, state.ref(), [&numPreCommitCalls](cache::CatapultCacheDelta& cacheDelta) {
			// Assert: there should be 2 known accounts
			EXPECT_EQ(2u, cacheDelta.sub<cache::AccountStateCache>().size());
			++numPreCommitCalls;
		});

		// Assert:
		EXPECT_EQ(1u, numPreCommitCalls);
	}

	// endregion
}}
