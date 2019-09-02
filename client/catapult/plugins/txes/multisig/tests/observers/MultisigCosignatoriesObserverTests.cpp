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

#include "src/observers/Observers.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MultisigCosignatoriesObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(MultisigCosignatories,)

	namespace {
		constexpr auto Add = model::CosignatoryModificationAction::Add;
		constexpr auto Del = model::CosignatoryModificationAction::Del;
		using ObserverTestContext = test::ObserverTestContextT<test::MultisigCacheFactory>;
		using Notification = model::MultisigCosignatoriesNotification;
		using Modifications = std::vector<model::CosignatoryModification>;

		auto CreateNotification(const Key& signerKey, const std::vector<model::CosignatoryModification>& modifications) {
			return Notification(signerKey, static_cast<uint8_t>(modifications.size()), modifications.data());
		}

		void LinkMultisigWithCosignatory(
				cache::MultisigCacheDelta& cache,
				const Key& multisigAccountKey,
				const Key& cosignatoryAccountKey) {
			if (!cache.contains(cosignatoryAccountKey))
				cache.insert(state::MultisigEntry(cosignatoryAccountKey));

			cache.find(cosignatoryAccountKey).get().multisigPublicKeys().insert(multisigAccountKey);
			cache.find(multisigAccountKey).get().cosignatoryPublicKeys().insert(cosignatoryAccountKey);
		}

		class MultisigCacheFacade {
		public:
			explicit MultisigCacheFacade(cache::MultisigCacheDelta& multisigCache) : m_multisigCache(multisigCache)
			{}

		public:
			void linkMultisigEntryWithCosignatories(const Key& multisigAccountKey, const std::vector<Key>& cosignatoryKeys) {
				if (!m_multisigCache.contains(multisigAccountKey))
					m_multisigCache.insert(state::MultisigEntry(multisigAccountKey));

				for (const auto& cosignatoryKey : cosignatoryKeys)
					LinkMultisigWithCosignatory(m_multisigCache, multisigAccountKey, cosignatoryKey);
			}

		public:
			void assertAccountsAreNotInMultisigCache(const std::vector<Key>& accountKeys) const {
				for (const auto& accountKey : accountKeys)
					assertNoMultisigEntryInCache(accountKey);
			}

			void assertHasCosignatories(const Key& accountKey, const std::vector<Key>& cosignatoryKeys) const {
				// Assert:
				ASSERT_TRUE(m_multisigCache.contains(accountKey)) << "cache is missing account " << accountKey;

				auto& multisigEntry = m_multisigCache.find(accountKey).get();
				assertAccountsInSet(cosignatoryKeys, multisigEntry.cosignatoryPublicKeys());
			}

			void assertHasMultisigAccounts(const Key& accountKey, const std::vector<Key>& multisigAccountKeys) const {
				// Assert:
				ASSERT_TRUE(m_multisigCache.contains(accountKey)) << "cache is missing account " << accountKey;

				auto& multisigEntry = m_multisigCache.find(accountKey).get();
				assertAccountsInSet(multisigAccountKeys, multisigEntry.multisigPublicKeys());
			}

		private:
			void assertNoMultisigEntryInCache(const Key& accountKey) const {
				EXPECT_FALSE(m_multisigCache.contains(accountKey)) << "cache should not have account " << accountKey;
			}

			void assertAccountsInSet(const std::vector<Key>& expectedKeys, const utils::SortedKeySet& keys) const {
				// Assert:
				for (const auto& key : expectedKeys)
					EXPECT_TRUE(m_multisigCache.contains(key)) << "cache is missing account " << key;

				test::AssertContents(expectedKeys, keys);
			}

		private:
			cache::MultisigCacheDelta& m_multisigCache;
		};

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunTest(
				const Notification& notification,
				ObserverTestContext&& context,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateMultisigCosignatoriesObserver();

			// - seed the cache
			MultisigCacheFacade cacheFacade(context.cache().sub<cache::MultisigCache>());
			seedCache(cacheFacade);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(cacheFacade);
		}

		struct MultisigDescriptor {
		public:
			MultisigDescriptor(size_t accountId, const std::vector<size_t>& cosignatories)
					: MultisigId(accountId)
					, CosignatoryIds(cosignatories)
			{}

		public:
			size_t MultisigId;
			std::vector<size_t> CosignatoryIds;
		};

		auto IdsToKeys(const std::vector<Key>& keys, const std::vector<size_t>& ids) {
			std::vector<Key> mapped;
			for (auto id : ids)
				mapped.push_back(keys[id]);

			return mapped;
		}

		void InitMultisigTest(
				MultisigCacheFacade& cacheFacade,
				const std::vector<Key>& keys,
				const std::vector<size_t>& unknownAccounts,
				const std::vector<MultisigDescriptor>& multisigAccounts) {
			for (const auto& descriptor : multisigAccounts)
				cacheFacade.linkMultisigEntryWithCosignatories(keys[descriptor.MultisigId], IdsToKeys(keys, descriptor.CosignatoryIds));

			// Sanity: verify initial state
			cacheFacade.assertAccountsAreNotInMultisigCache(IdsToKeys(keys, unknownAccounts));
		}

		void AssertMultisigTestResults(
				const MultisigCacheFacade& cacheFacade,
				const std::vector<Key>& keys,
				const std::vector<size_t>& unknownAccounts,
				const std::vector<MultisigDescriptor>& multisigAccounts) {
			cacheFacade.assertAccountsAreNotInMultisigCache(IdsToKeys(keys, unknownAccounts));

			std::map<size_t, std::vector<size_t>> cosignatoryMultisigs;
			for (const auto& descriptor : multisigAccounts) {
				cacheFacade.assertHasCosignatories(keys[descriptor.MultisigId], IdsToKeys(keys, descriptor.CosignatoryIds));
				for (auto cosignatoryId : descriptor.CosignatoryIds)
					cosignatoryMultisigs[cosignatoryId].push_back(descriptor.MultisigId);
			}

			for (const auto& entry : cosignatoryMultisigs)
				cacheFacade.assertHasMultisigAccounts(keys[entry.first], IdsToKeys(keys, entry.second));
		}

		struct CommitTraits {
		public:
			static void RunMultisigTest(
					const std::vector<Key>& keys,
					const std::vector<size_t>& initialUnknownAccounts,
					const std::vector<MultisigDescriptor>& initialMultisigAccounts,
					const Notification& notification,
					const std::vector<size_t>& finalUnknownAccounts,
					const std::vector<MultisigDescriptor>& finalMultisigAccounts) {

				RunTest(
						notification,
						ObserverTestContext(NotifyMode::Commit, Height(777)),
						[&keys, &initialUnknownAccounts, &initialMultisigAccounts](auto& cacheFacade) {
							InitMultisigTest(cacheFacade, keys, initialUnknownAccounts, initialMultisigAccounts);
						},
						[&keys, &finalUnknownAccounts, &finalMultisigAccounts](const auto& cacheFacade) {
							AssertMultisigTestResults(cacheFacade, keys, finalUnknownAccounts, finalMultisigAccounts);
						});
			}
		};

		struct RollbackTraits {
		public:
			static void RunMultisigTest(
					const std::vector<Key>& keys,
					const std::vector<size_t>& initialUnknownAccounts,
					const std::vector<MultisigDescriptor>& initialMultisigAccounts,
					const Notification& notification,
					const std::vector<size_t>& finalUnknownAccounts,
					const std::vector<MultisigDescriptor>& finalMultisigAccounts) {

				RunTest(
						notification,
						ObserverTestContext(NotifyMode::Rollback, Height(777)),
						[&keys, &finalUnknownAccounts, &finalMultisigAccounts](auto& cacheFacade) {
							InitMultisigTest(cacheFacade, keys, finalUnknownAccounts, finalMultisigAccounts);
						},
						[&keys, &initialUnknownAccounts, &initialMultisigAccounts](const auto& cacheFacade) {
							AssertMultisigTestResults(cacheFacade, keys, initialUnknownAccounts, initialMultisigAccounts);
						});
			}
		};
	}

#define NOTIFY_MODE_BASED_TRAITS(TEST_NAME) \
	template<typename TTraits> void TEST_NAME(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TEST_NAME<CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TEST_NAME<RollbackTraits>(); } \
	template<typename TTraits> void TEST_NAME()

	// region add

	NOTIFY_MODE_BASED_TRAITS(CanAddCosignatories) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Add, keys[1] }, { Add, keys[2] }, { Add, keys[3] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, { 0, 1, 2, 3 }, {}, notification, {}, {{ 0, { 1, 2, 3 } }});
	}

	NOTIFY_MODE_BASED_TRAITS(CanConvertCosignatoryToMultisigAccount) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Add, keys[3] }, { Add, keys[4] } };
		auto notification = CreateNotification(keys[1], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, { 3, 4 }, {{ 0, { 1, 2 } }}, notification, {}, {{ 0, { 1, 2 } }, { 1, { 3, 4 } }, });
	}

	NOTIFY_MODE_BASED_TRAITS(CanAddMultisigAccountAsACosignatory) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Add, keys[1] }, { Add, keys[4] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, { 0, 4 }, {{ 1, { 2, 3 } }}, notification, {}, {{ 0, { 1, 4 } }, { 1, { 2, 3 } }, });
	}

	// endregion

	// region add and delete

	NOTIFY_MODE_BASED_TRAITS(CanAddAndRemoveCosignatories) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Add, keys[1] }, { Del, keys[2] }, { Del, keys[3] }, { Add, keys[4] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, { 1, 4 }, {{ 0, { 2, 3 } }}, notification, { 2, 3 }, {{ 0, { 1, 4 } }});
	}

	// endregion

	// region remove if empty

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_RemovesMultisigAccountWithNoLinks) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Del, keys[1] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, {}, {{ 0, { 1 } }}, notification, { 0, 1 }, {});
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_LeavesMultisigAccountWithCosignatories) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Del, keys[2] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, {}, {{ 0, { 1, 2 } }}, notification, { 2 }, {{ 0, { 1 } }});
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_LeavesMultisigAccountWithMultisigAccounts) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Del, keys[2] } };
		auto notification = CreateNotification(keys[1], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, {}, {{ 0, { 1 } }, { 1, { 2 } }}, notification, { 2 }, {{ 0, { 1 } }});
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_LeavesCosignatoryWithCosignatories_RemovesMultisigAccountWitNoLinks) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Del, keys[1] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, {}, {{ 0, { 1 } }, { 1, { 2 } }}, notification, { 0 }, {{ 1, { 2 } }});
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_LeavesCosignatoryWithMultisigAccounts_RemovesMultisigAccountWithNoLinks) {
		// Arrange:
		auto keys = test::GenerateKeys(10);
		Modifications modifications = { { Del, keys[2] } };
		auto notification = CreateNotification(keys[0], modifications);

		// Act + Assert:
		TTraits::RunMultisigTest(keys, {}, {{ 0, { 2 } }, { 1, { 2 } }}, notification, { 0 }, {{ 1, { 2 } }});
	}

	// endregion
}}
