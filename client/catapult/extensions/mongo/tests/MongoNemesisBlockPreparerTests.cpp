#include "mongo/src/MongoNemesisBlockPreparer.h"
#include "mongo/src/ExternalCacheStorage.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/core/mocks/MockBlockStorage.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nemesis/NemesisTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoNemesisBlockPreparerTests

	namespace {
		using AccountStateAddresses = model::AddressSet;

		// region MockExternalAccountStateCacheStorage

		class MockExternalAccountStateCacheStorage final : public mongo::ExternalCacheStorageT<cache::AccountStateCache> {
		private:
			using LoadCheckpointFunc = typename mongo::ExternalCacheStorageT<cache::AccountStateCache>::LoadCheckpointFunc;

		public:
			MockExternalAccountStateCacheStorage() : m_numSaveDeltaCalls(0)
			{}

		private:
			void saveDelta(const cache::AccountStateCacheDelta& delta) override {
				++m_numSaveDeltaCalls;

				for (const auto* pAccountState : delta.addedElements())
					m_addedAccountStateAddresses.insert(pAccountState->Address);

				for (const auto* pAccountState : delta.removedElements())
					m_removedAccountStateAddresses.insert(pAccountState->Address);
			}

			void loadAll(cache::AccountStateCacheDelta&, Height, const LoadCheckpointFunc&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
			}

		public:
			const AccountStateAddresses& addedAccountStateAddresses() const {
				return m_addedAccountStateAddresses;
			}

			const AccountStateAddresses& removedAccountStateAddresses() const {
				return m_removedAccountStateAddresses;
			}

		public:
			size_t numSaveDeltaCalls() const {
				return m_numSaveDeltaCalls;
			}

		private:
			AccountStateAddresses m_addedAccountStateAddresses;
			AccountStateAddresses m_removedAccountStateAddresses;
			size_t m_numSaveDeltaCalls;
		};

		// endregion

		// region TestContext

		model::BlockChainConfiguration CreateBlockChainConfiguration() {
			return test::LoadLocalNodeConfigurationWithNemesisPluginExtensions("").BlockChain;
		}

		class TestContext {
		public:
			TestContext() : TestContext(Height())
			{}

			explicit TestContext(Height externalStorageHeight)
					: m_pPluginManager(test::CreateDefaultPluginManager(CreateBlockChainConfiguration()))
					, m_cache(m_pPluginManager->createCache())
					, m_mongoStorage(externalStorageHeight)
			{}

		public:
			auto cacheView() const {
				return m_cache.createView();
			}

			auto sourceStorageHeight() const {
				return m_sourceStorage.chainHeight();
			}

			const auto& mongoStorageView() const {
				return m_mongoStorage;
			}

			const auto& externalCacheStorage() const {
				return m_externalCacheStorage;
			}

		public:
			bool prepare() {
				MongoNemesisBlockPreparer nemesisBlockPreparer(
						m_mongoStorage,
						m_externalCacheStorage,
						m_pPluginManager->config(),
						m_sourceStorage,
						*m_pPluginManager);

				return nemesisBlockPreparer.prepare(m_cache);
			}

		private:
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			cache::CatapultCache m_cache;
			mocks::MockMemoryBasedStorage m_sourceStorage;

			mocks::MockSavingBlockStorage m_mongoStorage;
			MockExternalAccountStateCacheStorage m_externalCacheStorage;
		};

		// endregion
	}

	// region basic nemesis loading

	TEST(TEST_CLASS, PrepareDoesNotChangeCacheState) {
		// Arrange:
		TestContext context;

		// Act:
		auto isPreparePerformed = context.prepare();

		// Assert:
		EXPECT_TRUE(isPreparePerformed);
		EXPECT_EQ(Height(0), context.cacheView().height());
	}

	TEST(TEST_CLASS, PrepareAugmentsAndSavesNemesisInDestinationStorage) {
		// Arrange:
		TestContext context;
		const auto& storage = context.mongoStorageView();

		// Sanity:
		EXPECT_TRUE(storage.savedBlockElements().empty());

		// Act:
		auto isPreparePerformed = context.prepare();

		// Assert:
		EXPECT_TRUE(isPreparePerformed);

		// - the block is saved to the the mongo storage
		ASSERT_EQ(1u, storage.savedBlockElements().size());

		// - the saved block has extracted addresses
		const auto& nemesisBlockElement = storage.savedBlockElements()[0];
		EXPECT_NE(0u, nemesisBlockElement.Transactions.size());
		for (const auto& transactionElement : nemesisBlockElement.Transactions)
			EXPECT_TRUE(!!transactionElement.OptionalExtractedAddresses);
	}

	// endregion

	// region cache storage forwarding

	namespace {
		bool ContainsAddress(const AccountStateAddresses& addedAccountStates, const Address& address) {
			return addedAccountStates.cend() != addedAccountStates.find(address);
		}

		bool ContainsModifiedPrivate(const AccountStateAddresses& addedAccountStates, const char* privateKeyString) {
			return ContainsAddress(addedAccountStates, test::RawPrivateKeyToAddress(privateKeyString));
		}

		bool ContainsModifiedPublic(const AccountStateAddresses& addedAccountStates, const char* publicKeyString) {
			return ContainsAddress(addedAccountStates, test::RawPublicKeyToAddress(publicKeyString));
		}
	}

	TEST(TEST_CLASS, PrepareForwardsAllNemesisAccountsToCacheStorage) {
		// Arrange:
		TestContext context;

		// Act:
		auto isPreparePerformed = context.prepare();

		// Assert:
		EXPECT_TRUE(isPreparePerformed);

		const auto& addedAccountStateAddresses = context.externalCacheStorage().addedAccountStateAddresses();
		EXPECT_EQ(3u + CountOf(test::Mijin_Test_Private_Keys), addedAccountStateAddresses.size());
		EXPECT_EQ(0u, context.externalCacheStorage().removedAccountStateAddresses().size());

		// - check nemesis and rental fee sinks
		EXPECT_TRUE(ContainsModifiedPrivate(addedAccountStateAddresses, test::Mijin_Test_Nemesis_Private_Key));
		EXPECT_TRUE(ContainsModifiedPublic(addedAccountStateAddresses, test::Namespace_Rental_Fee_Sink_Public_Key));
		EXPECT_TRUE(ContainsModifiedPublic(addedAccountStateAddresses, test::Mosaic_Rental_Fee_Sink_Public_Key));

		// - check recipient accounts
		for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys)
			EXPECT_TRUE(ContainsModifiedPrivate(addedAccountStateAddresses, pRecipientPrivateKeyString)) << pRecipientPrivateKeyString;
	}

	// endregion

	// region conditional preparation

	namespace {
		void AssertNemesisBlockPreparation(Height externalStorageHeight, bool expectIsPreparePerformed) {
			// Arrange:
			TestContext context(externalStorageHeight);

			// Sanity: the source should always have a height of one
			EXPECT_EQ(Height(1), context.sourceStorageHeight());

			// Act:
			auto isPreparePerformed = context.prepare();

			// Assert:
			EXPECT_EQ(expectIsPreparePerformed, isPreparePerformed);

			auto numExpectedSaves = expectIsPreparePerformed ? 1u : 0u;
			EXPECT_EQ(numExpectedSaves, context.externalCacheStorage().numSaveDeltaCalls());
			EXPECT_EQ(numExpectedSaves, context.mongoStorageView().savedBlockElements().size());
		}
	}

	TEST(TEST_CLASS, PreparationIsPerformedWhenExternalStorageHeightIsZero) {
		// Assert:
		AssertNemesisBlockPreparation(Height(), true);
	}

	TEST(TEST_CLASS, PreparationIsSkippedWhenExternalStorageHeightIsOne) {
		// Assert:
		AssertNemesisBlockPreparation(Height(1), false);
	}

	TEST(TEST_CLASS, PreparationIsSkippedWhenExternalStorageHeightIsGreaterThanOne) {
		// Assert:
		AssertNemesisBlockPreparation(Height(2), false);
		AssertNemesisBlockPreparation(Height(10), false);
		AssertNemesisBlockPreparation(Height(1234), false);
	}

	// endregion
}}
