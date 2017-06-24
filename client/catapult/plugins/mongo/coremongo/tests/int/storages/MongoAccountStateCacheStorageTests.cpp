#include "src/storages/MongoAccountStateCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		using AccountStatePointer = std::shared_ptr<const state::AccountState>;
		using AccountStates = std::unordered_set<AccountStatePointer>;
		using MutableAccountStates = std::unordered_set<std::shared_ptr<state::AccountState>>;

		auto GenerateRandomAccountState(Height height) {
			auto publicKey = test::GenerateRandomData<Key_Size>();
			auto pState = std::make_shared<state::AccountState>(
					model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Mijin_Test),
					Height(1234567) + height);
			pState->PublicKey = publicKey;
			pState->PublicKeyHeight = Height(1234567) + height;
			auto randomAmount = Amount((test::Random() % 1'000'000 + 1'000) * 1'000'000);
			auto randomImportance = Importance(test::Random() % 1'000'000'000 + 1'000'000'000);
			auto randomImportanceHeight = test::GenerateRandomValue<model::ImportanceHeight>();
			pState->Balances.credit(Xem_Id, randomAmount);
			pState->ImportanceInfo.set(randomImportance, randomImportanceHeight);
			return pState;
		}

		auto GenerateAccountStates(size_t count) {
			MutableAccountStates states;
			for (auto i = 0u; i < count; ++i)
				states.insert(GenerateRandomAccountState(Height(i)));

			return states;
		}

		auto MakeConstCopy(const MutableAccountStates& states) {
			AccountStates result;
			for (auto& pState : states)
				result.insert(pState);

			return result;
		}

		void MutateAccountState(state::AccountState& state) {
			state.Balances.credit(Xem_Id, Amount(12'345'000'000u));
		}

		auto CreateEmptyCatapultCache() {
			auto chainConfig = model::BlockChainConfiguration::Uninitialized();
			chainConfig.Network.Identifier = Network_Identifier;
			return test::CreateEmptyCatapultCache(chainConfig);
		}

		void AddToCatapultCacheDelta(cache::CatapultCacheDelta& delta, const AccountStates& accountStates) {
			auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
			for (const AccountStatePointer& pAccountState : accountStates) {
				auto pState = accountStateCacheDelta.addAccount(pAccountState->PublicKey, pAccountState->PublicKeyHeight);
				pState->Balances.credit(Xem_Id, pAccountState->Balances.get(Xem_Id));
				auto height = pAccountState->ImportanceInfo.height();
				pState->ImportanceInfo.set(pAccountState->ImportanceInfo.get(height), height);
			}
		}

		void RemoveFromCatapultCacheDelta(cache::CatapultCacheDelta& delta, const AccountStates& accountStates) {
			auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
			for (const AccountStatePointer& pAccountState : accountStates)
				accountStateCacheDelta.queueRemove(pAccountState->PublicKey, pAccountState->PublicKeyHeight);

			accountStateCacheDelta.commitRemovals();
		}

		void AssertDbAccountStates(const AccountStates& accountStates) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			auto cursor = database["accounts"].find({});
			auto numAccounts = std::distance(cursor.begin(), cursor.end());

			ASSERT_EQ(accountStates.size(), numAccounts);

			for (const auto& pAccountState : accountStates) {
				auto filter = document{} << "account.address" << mappers::ToBinary(pAccountState->Address) << finalize;
				auto matchedDocument = database["accounts"].find_one(filter.view());
				ASSERT_TRUE(matchedDocument.is_initialized());

				auto account = matchedDocument.get().view()["account"].get_document();
				test::AssertEqualAccountState(*pAccountState, account.view());
			}
		}

		class TestAccountStatesContext : public test::PrepareDatabaseMixin {
		public:
			TestAccountStatesContext()
					: PrepareDatabaseMixin(test::DatabaseName())
					, m_cache(CreateEmptyCatapultCache())
					, m_pConfig(test::CreateDefaultMongoStorageConfiguration(test::DatabaseName()))
					, m_pCacheStorage(CreateMongoAccountStateCacheStorage(m_pConfig->createDatabaseConnection(), m_pConfig->bulkWriter()))
			{}

			void saveAccountStates(const AccountStates& modifiedAccountStates, const AccountStates& removedAccountStates = {}) {
				{
					// in order for an account to be removed from the cache, it must first exist in the cache
					auto delta = m_cache.createDelta();
					AddToCatapultCacheDelta(delta, removedAccountStates);
					m_cache.commit(Height(1));
				}

				auto delta = m_cache.createDelta();
				AddToCatapultCacheDelta(delta, modifiedAccountStates);
				RemoveFromCatapultCacheDelta(delta, removedAccountStates);
				m_pCacheStorage->saveDelta(delta);
			}

			MutableAccountStates loadAccountStates() {
				MutableAccountStates loadedAccountStates;
				m_pCacheStorage->loadAll(m_cache, Height(1));
				auto& subCache = m_cache.sub<cache::AccountStateCache>();
				auto view = subCache.createView();
				for (auto iter = view->cbegin(); view->cend() != iter; ++iter)
					loadedAccountStates.insert(iter->second);

				return loadedAccountStates;
			}

		private:
			cache::CatapultCache m_cache;
			std::shared_ptr<plugins::MongoStorageConfiguration> m_pConfig;
			std::unique_ptr<plugins::ExternalCacheStorage> m_pCacheStorage;
		};

		void AssertAccountStates(const MutableAccountStates& expectedStates, const MutableAccountStates& actualStates) {
			ASSERT_EQ(expectedStates.size(), actualStates.size());

			std::map<Address, std::shared_ptr<state::AccountState>> resultMap;
			for (const auto& pAccountState : actualStates)
				resultMap.emplace(pAccountState->Address, pAccountState);

			for (const auto& pExpectedAccountState : expectedStates) {
				auto iter = resultMap.find(pExpectedAccountState->Address);
				ASSERT_NE(resultMap.cend(), iter) << "for address " << test::ToHexString(pExpectedAccountState->Address);

				const auto& pAccountState = iter->second;
				test::AssertEqual(*pExpectedAccountState, *pAccountState);
			}
		}
	}

	TEST(MongoAccountStateCacheStorageTests, CanAddAccountState) {
		// Arrange: seed the cache with a single account
		TestAccountStatesContext context;
		auto pOriginal = GenerateRandomAccountState(Height(12345));
		context.saveAccountStates({ pOriginal });

		// Sanity:
		AssertDbAccountStates({ pOriginal });

		// Act:
		auto pAccountState = GenerateRandomAccountState(Height(54321));
		context.saveAccountStates({ pAccountState });

		// Assert:
		AssertDbAccountStates({ pOriginal, pAccountState });
	}

	TEST(MongoAccountStateCacheStorageTests, CanUpdateAccountState) {
		// Arrange: seed the cache with a single account
		TestAccountStatesContext context;
		auto pOriginal = GenerateRandomAccountState(Height(12345));
		context.saveAccountStates({ pOriginal });

		// Sanity:
		AssertDbAccountStates({ pOriginal });

		// Act:
		MutateAccountState(*pOriginal);
		context.saveAccountStates({ pOriginal });

		// Assert:
		AssertDbAccountStates({ pOriginal });
	}

	TEST(MongoAccountStateCacheStorageTests, CanRemoveAccountState) {
		// Arrange: seed the cache with two accounts
		TestAccountStatesContext context;
		auto pOriginal = GenerateRandomAccountState(Height(12345));
		auto pAccountToRemove = GenerateRandomAccountState(Height(54321));
		context.saveAccountStates({ pOriginal, pAccountToRemove });

		// Sanity:
		AssertDbAccountStates({ pOriginal, pAccountToRemove });

		// Act:
		context.saveAccountStates({}, { pAccountToRemove });

		// Assert:
		AssertDbAccountStates({ pOriginal });
	}

	TEST(MongoAccountStateCacheStorageTests, CanSaveAccountStates) {
		// Arrange:
		TestAccountStatesContext context;
		auto constAccountStates = MakeConstCopy(GenerateAccountStates(100));

		// Act:
		context.saveAccountStates(constAccountStates);

		// Assert:
		AssertDbAccountStates(constAccountStates);
	}

	TEST(MongoAccountStateCacheStorageTests, CanUpdateAccountStates) {
		// Arrange:
		TestAccountStatesContext context;
		auto accountStates = GenerateAccountStates(100);
		context.saveAccountStates(MakeConstCopy(accountStates));

		// Act: drop some and modify some
		AccountStates notRemoved;
		AccountStates removed;
		AccountStates modified;
		enum class Action { Remove, Modify, Add };
		for (auto& pState : accountStates) {
			Action action = static_cast<Action>(pState->PublicKeyHeight.unwrap() % 3);

			if (Action::Remove == action) {
				removed.insert(pState);
				continue;
			}

			notRemoved.insert(pState);
			if (Action::Modify == action) {
				MutateAccountState(*pState);
				modified.insert(pState);
			}
		}

		// Sanity:
		EXPECT_FALSE(removed.empty());
		EXPECT_FALSE(modified.empty());
		EXPECT_LT(modified.size(), notRemoved.size());

		// Act:
		context.saveAccountStates(modified, removed);

		// Assert:
		AssertDbAccountStates(notRemoved);
	}

	TEST(MongoAccountStateCacheStorageTests, CanLoadAccountStates) {
		// Arrange:
		TestAccountStatesContext context;
		auto originalAccountStates = GenerateAccountStates(100);
		context.saveAccountStates(MakeConstCopy(originalAccountStates));

		// Act:
		auto accountStates = context.loadAccountStates();

		// Assert:
		AssertAccountStates(originalAccountStates, accountStates);
	}
}}}
