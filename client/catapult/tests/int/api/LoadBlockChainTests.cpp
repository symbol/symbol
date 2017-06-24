#include "plugins/mongo/coremongo/src/ExternalCacheStorage.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/local/api/LocalNode.h"
#include "catapult/model/Address.h"
#include "catapult/state/AccountState.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/int/Configuration.h"
#include "tests/test/int/NemesisTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/mocks/MockChainScoreProvider.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		using AccountStateAddresses = std::unordered_set<Address, utils::ArrayHasher<Address>>;

		// region MockExternalAccountStateCacheStorage

		class MockExternalAccountStateCacheStorage final : public mongo::plugins::ExternalCacheStorageT<cache::AccountStateCache> {
		private:
			using LoadCheckpointFunc = typename mongo::plugins::ExternalCacheStorageT<cache::AccountStateCache>::LoadCheckpointFunc;

		private:
			void saveDelta(const cache::AccountStateCacheDelta& delta) override {
				for (const auto& pAccountState : delta.modifiedAccountStates())
					m_modifiedAccountStateAddresses.insert(pAccountState->Address);

				for (const auto& pAccountState : delta.removedAccountStates())
					m_removedAccountStateAddresses.insert(pAccountState->Address);
			}

			void loadAll(cache::AccountStateCacheDelta&, Height, const LoadCheckpointFunc&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

		public:
			const AccountStateAddresses& modifiedAccountStateAddresses() const {
				return m_modifiedAccountStateAddresses;
			}

			const AccountStateAddresses& removedAccountStateAddresses() const {
				return m_removedAccountStateAddresses;
			}

		private:
			AccountStateAddresses m_modifiedAccountStateAddresses;
			AccountStateAddresses m_removedAccountStateAddresses;
		};

		// endregion

		// region TestContext

		class TestContext {
		public:
			TestContext() : m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				auto config = test::LoadLocalNodeConfigurationWithNemesisPluginExtensions(m_tempDir.name());
				m_expectedHistorySize = model::CalculateDifficultyHistorySize(config.BlockChain);

				// note that this trick only works for MemoryBasedStorage, which allows the nemesis block to be dropped
				// (by causing storage to have height 0, local node will load nemesis from disk and save it into storage)
				auto pStorage = std::make_unique<mocks::MemoryBasedStorage>();
				pStorage->dropBlocksAfter(Height(0));

				auto pExternalCacheStorage = std::make_unique<MockExternalAccountStateCacheStorage>();
				m_pRawExternalCacheStorage = pExternalCacheStorage.get();

				auto pUnconfirmedTransactionsCache = test::CreateUnconfirmedTransactionsCache();
				auto viewProvider = test::CreateViewProvider(*pUnconfirmedTransactionsCache);
				m_pLocalNode = CreateLocalNode(
						m_serverKeyPair,
						std::move(config),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeTests-api"),
						std::move(pStorage),
						nullptr,
						std::move(pExternalCacheStorage),
						viewProvider,
						std::move(pUnconfirmedTransactionsCache));
			}

		public:
			const auto& cache() {
				return m_pLocalNode->cache();
			}

			auto score() {
				return m_pLocalNode->score();
			}

			const auto& externalCacheStorage() {
				return *m_pRawExternalCacheStorage;
			}

		private:
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			uint64_t m_expectedHistorySize;
			MockExternalAccountStateCacheStorage* m_pRawExternalCacheStorage;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};

		// endregion
	}

	// region basic nemesis loading

	TEST(LoadBlockChainTests, ProperAccountStateAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		const auto& view = context.cache().createView();
		EXPECT_EQ(Height(1), view.height());
		test::AssertNemesisAccountState(view);
	}

	TEST(LoadBlockChainTests, ProperMosaicStateAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		const auto& view = context.cache().createView();
		test::AssertNemesisNamespaceState(view);
		test::AssertNemesisMosaicState(view);
	}

	TEST(LoadBlockChainTests, ProperChainScoreAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		EXPECT_EQ(model::ChainScore(), context.score());
	}

	// endregion

	namespace {
		bool ContainsAddress(const AccountStateAddresses& modifiedAccountStates, const Address& address) {
			return modifiedAccountStates.cend() != modifiedAccountStates.find(address);
		}

		bool ContainsModifiedPrivate(const AccountStateAddresses& modifiedAccountStates, const char* pPrivateKeyString) {
			return ContainsAddress(modifiedAccountStates, test::RawPrivateKeyToAddress(pPrivateKeyString));
		}

		bool ContainsModifiedPublic(const AccountStateAddresses& modifiedAccountStates, const char* pPublicKeyString) {
			return ContainsAddress(modifiedAccountStates, test::RawPublicKeyToAddress(pPublicKeyString));
		}
	}

	TEST(LoadBlockChainTests, AllNemesisAccountsAreForwardedToCacheStorage) {
		// Act:
		TestContext context;

		// Assert:
		const auto& storage = context.externalCacheStorage();
		const auto& modifiedAccountStateAddresses = storage.modifiedAccountStateAddresses();
		EXPECT_EQ(3u + CountOf(test::Mijin_Test_Private_Keys), modifiedAccountStateAddresses.size());
		EXPECT_EQ(0u, storage.removedAccountStateAddresses().size());

		// - check nemesis and rental fee sinks
		EXPECT_TRUE(ContainsModifiedPrivate(modifiedAccountStateAddresses, test::Mijin_Test_Nemesis_Private_Key));
		EXPECT_TRUE(ContainsModifiedPublic(modifiedAccountStateAddresses, test::Namespace_Rental_Fee_Sink_Public_Key));
		EXPECT_TRUE(ContainsModifiedPublic(modifiedAccountStateAddresses, test::Mosaic_Rental_Fee_Sink_Public_Key));

		// - check recipient accounts
		for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys)
			EXPECT_TRUE(ContainsModifiedPrivate(modifiedAccountStateAddresses, pRecipientPrivateKeyString)) << pRecipientPrivateKeyString;
	}
}}}
