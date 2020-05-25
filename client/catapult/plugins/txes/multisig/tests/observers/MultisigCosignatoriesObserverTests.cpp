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
		using ObserverTestContext = test::ObserverTestContextT<test::MultisigCacheFactory>;
		using Notification = model::MultisigCosignatoriesNotification;

		constexpr auto CreateNotification = test::CreateMultisigCosignatoriesNotification;

		std::vector<UnresolvedAddress> ToUnresolvedAddresses(const std::vector<Address>& addresses) {
			std::vector<UnresolvedAddress> unresolvedAddresses;
			for (const auto& address : addresses)
				unresolvedAddresses.push_back(test::UnresolveXor(address));

			return unresolvedAddresses;
		}

		void LinkMultisigWithCosignatory(cache::MultisigCacheDelta& cache, const Address& multisig, const Address& cosignatory) {
			if (!cache.contains(cosignatory))
				cache.insert(state::MultisigEntry(cosignatory));

			cache.find(cosignatory).get().multisigAddresses().insert(multisig);
			cache.find(multisig).get().cosignatoryAddresses().insert(cosignatory);
		}

		class MultisigCacheFacade {
		public:
			explicit MultisigCacheFacade(cache::MultisigCacheDelta& multisigCache) : m_multisigCache(multisigCache)
			{}

		public:
			void linkMultisigEntryWithCosignatories(const Address& multisig, const std::vector<Address>& cosignatories) {
				if (!m_multisigCache.contains(multisig))
					m_multisigCache.insert(state::MultisigEntry(multisig));

				for (const auto& cosignatory : cosignatories)
					LinkMultisigWithCosignatory(m_multisigCache, multisig, cosignatory);
			}

		public:
			void assertAccountsAreNotInMultisigCache(const std::vector<Address>& addresses) const {
				for (const auto& address : addresses)
					assertNoMultisigEntryInCache(address);
			}

			void assertHasCosignatories(const Address& address, const std::vector<Address>& cosignatories) const {
				// Assert:
				ASSERT_TRUE(m_multisigCache.contains(address)) << "cache is missing account " << address;

				const auto& multisigEntry = m_multisigCache.find(address).get();
				assertAccountsInSet(cosignatories, multisigEntry.cosignatoryAddresses());
			}

			void assertHasMultisigAccounts(const Address& address, const std::vector<Address>& multisigs) const {
				// Assert:
				ASSERT_TRUE(m_multisigCache.contains(address)) << "cache is missing account " << address;

				const auto& multisigEntry = m_multisigCache.find(address).get();
				assertAccountsInSet(multisigs, multisigEntry.multisigAddresses());
			}

		private:
			void assertNoMultisigEntryInCache(const Address& address) const {
				EXPECT_FALSE(m_multisigCache.contains(address)) << "cache should not have account " << address;
			}

			void assertAccountsInSet(const std::vector<Address>& expectedAddresses, const state::SortedAddressSet& addresses) const {
				// Assert:
				for (const auto& address : expectedAddresses)
					EXPECT_TRUE(m_multisigCache.contains(address)) << "cache is missing account " << address;

				test::AssertContents(expectedAddresses, addresses);
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
			MultisigDescriptor(size_t accountShortId, const std::vector<size_t>& cosignatories)
					: MultisigId(accountShortId)
					, CosignatoryIds(cosignatories)
			{}

		public:
			size_t MultisigId;
			std::vector<size_t> CosignatoryIds;
		};

		auto Pick(const std::vector<Address>& addresses, const std::vector<size_t>& ids) {
			std::vector<Address> mapped;
			for (auto id : ids)
				mapped.push_back(addresses[id]);

			return mapped;
		}

		void InitMultisigTest(
				MultisigCacheFacade& cacheFacade,
				const std::vector<Address>& addresses,
				const std::vector<size_t>& unknownAccounts,
				const std::vector<MultisigDescriptor>& multisigAccounts) {
			for (const auto& descriptor : multisigAccounts) {
				cacheFacade.linkMultisigEntryWithCosignatories(
						addresses[descriptor.MultisigId],
						Pick(addresses, descriptor.CosignatoryIds));
			}

			// Sanity: verify initial state
			cacheFacade.assertAccountsAreNotInMultisigCache(Pick(addresses, unknownAccounts));
		}

		void AssertMultisigTestResults(
				const MultisigCacheFacade& cacheFacade,
				const std::vector<Address>& addresses,
				const std::vector<size_t>& unknownAccounts,
				const std::vector<MultisigDescriptor>& multisigAccounts) {
			cacheFacade.assertAccountsAreNotInMultisigCache(Pick(addresses, unknownAccounts));

			std::map<size_t, std::vector<size_t>> cosignatoryMultisigs;
			for (const auto& descriptor : multisigAccounts) {
				cacheFacade.assertHasCosignatories(addresses[descriptor.MultisigId], Pick(addresses, descriptor.CosignatoryIds));
				for (auto cosignatoryId : descriptor.CosignatoryIds)
					cosignatoryMultisigs[cosignatoryId].push_back(descriptor.MultisigId);
			}

			for (const auto& entry : cosignatoryMultisigs)
				cacheFacade.assertHasMultisigAccounts(addresses[entry.first], Pick(addresses, entry.second));
		}

		struct CommitTraits {
		public:
			static void RunMultisigTest(
					const std::vector<Address>& addresses,
					const std::vector<size_t>& initialUnknownAccounts,
					const std::vector<MultisigDescriptor>& initialMultisigAccounts,
					const Notification& notification,
					const std::vector<size_t>& finalUnknownAccounts,
					const std::vector<MultisigDescriptor>& finalMultisigAccounts) {
				RunTest(
						notification,
						ObserverTestContext(NotifyMode::Commit, Height(777)),
						[&addresses, &initialUnknownAccounts, &initialMultisigAccounts](auto& cacheFacade) {
							InitMultisigTest(cacheFacade, addresses, initialUnknownAccounts, initialMultisigAccounts);
						},
						[&addresses, &finalUnknownAccounts, &finalMultisigAccounts](const auto& cacheFacade) {
							AssertMultisigTestResults(cacheFacade, addresses, finalUnknownAccounts, finalMultisigAccounts);
						});
			}
		};

		struct RollbackTraits {
		public:
			static void RunMultisigTest(
					const std::vector<Address>& addresses,
					const std::vector<size_t>& initialUnknownAccounts,
					const std::vector<MultisigDescriptor>& initialMultisigAccounts,
					const Notification& notification,
					const std::vector<size_t>& finalUnknownAccounts,
					const std::vector<MultisigDescriptor>& finalMultisigAccounts) {
				RunTest(
						notification,
						ObserverTestContext(NotifyMode::Rollback, Height(777)),
						[&addresses, &finalUnknownAccounts, &finalMultisigAccounts](auto& cacheFacade) {
							InitMultisigTest(cacheFacade, addresses, finalUnknownAccounts, finalMultisigAccounts);
						},
						[&addresses, &initialUnknownAccounts, &initialMultisigAccounts](const auto& cacheFacade) {
							AssertMultisigTestResults(cacheFacade, addresses, initialUnknownAccounts, initialMultisigAccounts);
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
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = ToUnresolvedAddresses({ addresses[1], addresses[2], addresses[3] });
		auto addressDeletions = std::vector<UnresolvedAddress>();
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, { 0, 1, 2, 3 }, {}, notification, {}, { { 0, { 1, 2, 3 } } });
	}

	NOTIFY_MODE_BASED_TRAITS(CanConvertCosignatoryToMultisigAccount) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = ToUnresolvedAddresses({ addresses[3], addresses[4] });
		auto addressDeletions = std::vector<UnresolvedAddress>();
		auto notification = CreateNotification(addresses[1], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, { 3, 4 }, { { 0, { 1, 2 } } }, notification, {}, { { 0, { 1, 2 } }, { 1, { 3, 4 } } });
	}

	NOTIFY_MODE_BASED_TRAITS(CanAddMultisigAccountAsACosignatory) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = ToUnresolvedAddresses({ addresses[1], addresses[4] });
		auto addressDeletions = std::vector<UnresolvedAddress>();
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, { 0, 4 }, { { 1, { 2, 3 } } }, notification, {}, { { 0, { 1, 4 } }, { 1, { 2, 3 } } });
	}

	// endregion

	// region add and delete

	NOTIFY_MODE_BASED_TRAITS(CanAddAndRemoveCosignatories) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = ToUnresolvedAddresses({ addresses[1], addresses[4] });
		auto addressDeletions = ToUnresolvedAddresses({ addresses[2], addresses[3] });
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, { 1, 4 }, { { 0, { 2, 3 } } }, notification, { 2, 3 }, { { 0, { 1, 4 } } });
	}

	// endregion

	// region remove if empty

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_RemovesMultisigAccountWithNoLinks) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = std::vector<UnresolvedAddress>();
		auto addressDeletions = ToUnresolvedAddresses({ addresses[1] });
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, {}, { { 0, { 1 } } }, notification, { 0, 1 }, {});
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_LeavesMultisigAccountWithCosignatories) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = std::vector<UnresolvedAddress>();
		auto addressDeletions = ToUnresolvedAddresses({ addresses[2] });
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, {}, { { 0, { 1, 2 } } }, notification, { 2 }, { { 0, { 1 } } });
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_RemovesCosignatoryWithNoLinks_LeavesMultisigAccountWithMultisigAccounts) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = std::vector<UnresolvedAddress>();
		auto addressDeletions = ToUnresolvedAddresses({ addresses[2] });
		auto notification = CreateNotification(addresses[1], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, {}, { { 0, { 1 } }, { 1, { 2 } } }, notification, { 2 }, { { 0, { 1 } } });
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_LeavesCosignatoryWithCosignatories_RemovesMultisigAccountWitNoLinks) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = std::vector<UnresolvedAddress>();
		auto addressDeletions = ToUnresolvedAddresses({ addresses[1] });
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, {}, { { 0, { 1 } }, { 1, { 2 } } }, notification, { 0 }, { { 1, { 2 } } });
	}

	NOTIFY_MODE_BASED_TRAITS(RemovingCosignatory_LeavesCosignatoryWithMultisigAccounts_RemovesMultisigAccountWithNoLinks) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto addressAdditions = std::vector<UnresolvedAddress>();
		auto addressDeletions = ToUnresolvedAddresses({ addresses[2] });
		auto notification = CreateNotification(addresses[0], addressAdditions, addressDeletions);

		// Act + Assert:
		TTraits::RunMultisigTest(addresses, {}, { { 0, { 2 } }, { 1, { 2 } } }, notification, { 0 }, { { 1, { 2 } } });
	}

	// endregion
}}
