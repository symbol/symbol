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

#include "harvesting/src/UnlockedAccounts.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedAccountsTests

	// region test utils

	namespace {
		struct KeyPairsWrapper {
		public:
			BlockGeneratorKeyPairs KeyPairs;
			Key SigningPublicKey;
			Key VrfPublicKey;

		public:
			KeyPairsWrapper()
					: KeyPairs(test::GenerateKeyPair(), test::GenerateKeyPair())
					, SigningPublicKey(KeyPairs.signingKeyPair().publicKey())
					, VrfPublicKey(KeyPairs.vrfKeyPair().publicKey())
			{}
		};

		KeyPairsWrapper GenerateKeyPairsWrapper() {
			return KeyPairsWrapper();
		}

		struct TestContext {
		public:
			explicit TestContext(size_t maxSize) : Accounts(maxSize, [this](const auto& publicKey) {
				auto iter = CustomPrioritizationMap.find(publicKey);
				return CustomPrioritizationMap.cend() != iter ? iter->second : 0;
			})
			{}

		public:
			UnlockedAccounts Accounts;
			std::unordered_map<Key, size_t, utils::ArrayHasher<Key>> CustomPrioritizationMap;
		};

		UnlockedAccountsAddResult AddAccount(TestContext& context, BlockGeneratorKeyPairs&& keyPairs) {
			return context.Accounts.modifier().add(std::move(keyPairs));
		}

		UnlockedAccountsAddResult AddRandomAccount(TestContext& context) {
			return AddAccount(context, GenerateKeyPairsWrapper().KeyPairs);
		}
	}

	// endregion

	// region basic ctor, add, remove

	namespace {
		std::pair<Key, size_t> GetPrioritizedVrfPublicKey(const UnlockedAccounts& accounts, const Key& signingPublicKey) {
			auto index = 0u;
			auto vrfPublicKey = Key();
			accounts.view().forEach([&signingPublicKey, &index, &vrfPublicKey](const auto& keyPairs) {
				if (keyPairs.signingKeyPair().publicKey() != signingPublicKey) {
					++index;
					return true;
				}

				vrfPublicKey = keyPairs.vrfKeyPair().publicKey();
				return false;
			});

			return std::make_pair(vrfPublicKey, index);
		}

		Key GetVrfPublicKey(const UnlockedAccounts& accounts, const Key& signingPublicKey) {
			return GetPrioritizedVrfPublicKey(accounts, signingPublicKey).first;
		}
	}

	TEST(TEST_CLASS, InitiallyContainerIsEmpty) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		// Assert:
		EXPECT_EQ(0u, accounts.view().size());
	}

	TEST(TEST_CLASS, CanAddHarvestingEligibleAccount) {
		// Arrange:
		auto keyPairsWrapper = GenerateKeyPairsWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result = accounts.modifier().add(std::move(keyPairsWrapper.KeyPairs));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairsWrapper.SigningPublicKey));

		// - vrf public key is properly associated
		EXPECT_FALSE(view.contains(keyPairsWrapper.VrfPublicKey));
		EXPECT_EQ(keyPairsWrapper.VrfPublicKey, GetVrfPublicKey(accounts, keyPairsWrapper.SigningPublicKey));
	}

	TEST(TEST_CLASS, CanAddMultipleAccountsToContainer) {
		// Arrange:
		auto keyPairsWrapper1 = GenerateKeyPairsWrapper();
		auto keyPairsWrapper2 = GenerateKeyPairsWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		{
			auto modifier = accounts.modifier();
			modifier.add(std::move(keyPairsWrapper1.KeyPairs));
			modifier.add(std::move(keyPairsWrapper2.KeyPairs));
		}

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(2u, view.size());
		EXPECT_TRUE(view.contains(keyPairsWrapper1.SigningPublicKey));
		EXPECT_TRUE(view.contains(keyPairsWrapper2.SigningPublicKey));

		// - vrf public keys are properly associated
		EXPECT_FALSE(view.contains(keyPairsWrapper1.VrfPublicKey));
		EXPECT_FALSE(view.contains(keyPairsWrapper2.VrfPublicKey));
		EXPECT_EQ(keyPairsWrapper1.VrfPublicKey, GetVrfPublicKey(accounts, keyPairsWrapper1.SigningPublicKey));
		EXPECT_EQ(keyPairsWrapper2.VrfPublicKey, GetVrfPublicKey(accounts, keyPairsWrapper2.SigningPublicKey));
	}

	TEST(TEST_CLASS, AdditionOfAlreadyAddedAccountUpdatesVrfPublicKey) {
		// Arrange:
		auto signingKeyPair = test::GenerateKeyPair();
		auto vrfKeyPair1 = test::GenerateKeyPair();
		auto vrfKeyPair2 = test::GenerateKeyPair();

		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result1 = accounts.modifier().add(BlockGeneratorKeyPairs(test::CopyKeyPair(signingKeyPair), test::CopyKeyPair(vrfKeyPair1)));
		auto result2 = accounts.modifier().add(BlockGeneratorKeyPairs(test::CopyKeyPair(signingKeyPair), test::CopyKeyPair(vrfKeyPair2)));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result1);
		EXPECT_EQ(UnlockedAccountsAddResult::Success_Update, result2);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(signingKeyPair.publicKey()));

		// - last vrf public key is properly associated
		EXPECT_FALSE(view.contains(vrfKeyPair1.publicKey()));
		EXPECT_FALSE(view.contains(vrfKeyPair2.publicKey()));
		EXPECT_EQ(vrfKeyPair2.publicKey(), GetVrfPublicKey(accounts, signingKeyPair.publicKey()));
	}

	TEST(TEST_CLASS, AdditionOfAlreadyAddedAccountUpdatesVrfPublicKeyAndPreservesPriority) {
		// Arrange:
		auto signingKeyPair1 = test::GenerateKeyPair();
		auto signingKeyPair2 = test::GenerateKeyPair();
		auto vrfKeyPair1 = test::GenerateKeyPair();
		auto vrfKeyPair2 = test::GenerateKeyPair();

		TestContext context(8);
		auto& accounts = context.Accounts;

		// - add account 1 with priority 3
		context.CustomPrioritizationMap[signingKeyPair1.publicKey()] = 3;
		accounts.modifier().add(BlockGeneratorKeyPairs(test::CopyKeyPair(signingKeyPair1), test::CopyKeyPair(vrfKeyPair1)));

		// Act: lower account 1 priority and update VRF
		context.CustomPrioritizationMap[signingKeyPair1.publicKey()] = 1;
		auto result = accounts.modifier().add(BlockGeneratorKeyPairs(test::CopyKeyPair(signingKeyPair1), test::CopyKeyPair(vrfKeyPair2)));

		// - add account 2 with in-between priority
		context.CustomPrioritizationMap[signingKeyPair2.publicKey()] = 2;
		accounts.modifier().add(BlockGeneratorKeyPairs(test::CopyKeyPair(signingKeyPair2), test::GenerateKeyPair()));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_Update, result);
		EXPECT_EQ(2u, view.size());
		EXPECT_TRUE(view.contains(signingKeyPair1.publicKey()));
		EXPECT_TRUE(view.contains(signingKeyPair2.publicKey()));

		// - last vrf public key is properly associated and retains original (highest) priority
		EXPECT_FALSE(view.contains(vrfKeyPair1.publicKey()));
		EXPECT_FALSE(view.contains(vrfKeyPair2.publicKey()));

		const auto& vrfPrioritizedPublicKey = GetPrioritizedVrfPublicKey(accounts, signingKeyPair1.publicKey());
		EXPECT_EQ(vrfKeyPair2.publicKey(), vrfPrioritizedPublicKey.first);
		EXPECT_EQ(0u, vrfPrioritizedPublicKey.second);
	}

	TEST(TEST_CLASS, CanRemoveAccountFromContainer) {
		// Arrange:
		auto keyPairsWrapper1 = GenerateKeyPairsWrapper();
		auto keyPairsWrapper2 = GenerateKeyPairsWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		{
			auto modifier = accounts.modifier();
			modifier.add(std::move(keyPairsWrapper1.KeyPairs));
			modifier.add(std::move(keyPairsWrapper2.KeyPairs));
		}

		// Sanity:
		EXPECT_EQ(2u, accounts.view().size());

		// Act:
		auto removeResult = accounts.modifier().remove(keyPairsWrapper1.SigningPublicKey);

		// Assert:
		EXPECT_TRUE(removeResult);

		auto view = accounts.view();
		EXPECT_EQ(1u, view.size());
		EXPECT_FALSE(view.contains(keyPairsWrapper1.SigningPublicKey));
		EXPECT_TRUE(view.contains(keyPairsWrapper2.SigningPublicKey));
	}

	TEST(TEST_CLASS, RemovalOfAccountNotInContainerHasNoEffect) {
		// Arrange:
		auto keyPairsWrapper = GenerateKeyPairsWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result = accounts.modifier().add(std::move(keyPairsWrapper.KeyPairs));
		auto removeResult = accounts.modifier().remove(test::GenerateKeyPair().publicKey());

		// Assert:
		EXPECT_FALSE(removeResult);

		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairsWrapper.SigningPublicKey));
	}

	// endregion

	// region forEach iteration

	namespace {
		struct AccountPublicKeys {
		public:
			Key SigningPublicKey;
			Key VrfPublicKey;

		public:
			constexpr bool operator==(const AccountPublicKeys& rhs) const {
				return SigningPublicKey == rhs.SigningPublicKey && VrfPublicKey == rhs.VrfPublicKey;
			}
		};

		std::vector<AccountPublicKeys> AddAccounts(TestContext& context, size_t numAccounts) {
			std::vector<AccountPublicKeys> publicKeys;
			for (auto i = 0u; i < numAccounts; ++i) {
				auto keyPairsWrapper = GenerateKeyPairsWrapper();
				publicKeys.push_back({ keyPairsWrapper.SigningPublicKey, keyPairsWrapper.VrfPublicKey });
				AddAccount(context, std::move(keyPairsWrapper.KeyPairs));
			}

			return publicKeys;
		}

		std::vector<AccountPublicKeys> ExtractAllPublicKeysOrdered(
				const UnlockedAccountsView& view,
				size_t maxPublicKeys = std::numeric_limits<size_t>::max()) {
			std::vector<AccountPublicKeys> publicKeys;
			view.forEach([maxPublicKeys, &publicKeys](const auto& keyPairs) {
				publicKeys.push_back({ keyPairs.signingKeyPair().publicKey(), keyPairs.vrfKeyPair().publicKey() });
				return publicKeys.size() < maxPublicKeys;
			});

			return publicKeys;
		}
	}

	TEST(TEST_CLASS, CanIterateOverAllAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;
		auto expectedPublicKeys = AddAccounts(context, 4);

		// Act:
		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

		// Assert:
		EXPECT_EQ(4u, view.size());
		EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
	}

	TEST(TEST_CLASS, CanShortCircuitIterateOverAllAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		auto expectedPublicKeys = AddAccounts(context, 4);
		expectedPublicKeys.pop_back();
		expectedPublicKeys.pop_back();

		// Act:
		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view, 2);

		// Assert:
		EXPECT_EQ(4u, view.size());
		EXPECT_EQ(2u, actualPublicKeys.size());
		EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
	}

	TEST(TEST_CLASS, ForEachReturnsAccountsInStableDecreasingOrderOfPriority) {
		// Arrange:
		constexpr auto Num_Accounts = 12u;
		TestContext context(15);
		const auto& accounts = context.Accounts;

		// -     priorities: 2 1 0 2 1 0 2 1 0 2 1 0
		// - sorted indexes: 0 4 8 1 5 9 2 6 A 3 7 B
		std::vector<Key> expectedPublicKeys;
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto keyPairsWrapper = GenerateKeyPairsWrapper();
			expectedPublicKeys.push_back(keyPairsWrapper.SigningPublicKey);
			context.CustomPrioritizationMap.emplace(keyPairsWrapper.SigningPublicKey, 2 - (i % 3));
			AddAccount(context, std::move(keyPairsWrapper.KeyPairs));
		}

		// Act:
		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

		// Assert:
		EXPECT_EQ(Num_Accounts, view.size());
		EXPECT_EQ(Num_Accounts, actualPublicKeys.size());
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto expectedIndex = (i / 4) + 3 * (i % 4);
			EXPECT_EQ(expectedPublicKeys[expectedIndex], actualPublicKeys[i].SigningPublicKey)
					<< "expected index = " << expectedIndex
					<< ", actual index = " << i;
		}
	}

	TEST(TEST_CLASS, RemovedAccountsAreNotIterated) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;

		auto expectedPublicKeys = AddAccounts(context, 4);
		{
			auto modifier = accounts.modifier();
			modifier.remove((++expectedPublicKeys.cbegin())->SigningPublicKey);
			modifier.remove((--expectedPublicKeys.cend())->SigningPublicKey);
		}

		expectedPublicKeys.erase(++expectedPublicKeys.begin());
		expectedPublicKeys.erase(--expectedPublicKeys.cend());

		// Act:
		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

		// Assert:
		EXPECT_EQ(2u, view.size());
		EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
	}

	// endregion

	// region max accounts

	TEST(TEST_CLASS, CanAddMaxAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		// Act:
		for (auto i = 0u; i < 8; ++i)
			AddRandomAccount(context);

		// Assert:
		EXPECT_EQ(8u, accounts.view().size());
	}

	namespace {
		void AssertCannotAddMoreThanMaxAccounts(const std::function<size_t (size_t)>& indexToPriorityMap) {
			// Arrange:
			TestContext context(8);
			const auto& accounts = context.Accounts;

			std::vector<AccountPublicKeys> expectedPublicKeys;
			for (auto i = 0u; i < 8; ++i) {
				auto keyPairsWrapper = GenerateKeyPairsWrapper();
				expectedPublicKeys.push_back({ keyPairsWrapper.SigningPublicKey, keyPairsWrapper.VrfPublicKey });
				context.CustomPrioritizationMap.emplace(keyPairsWrapper.SigningPublicKey, indexToPriorityMap(i));
				AddAccount(context, std::move(keyPairsWrapper.KeyPairs));
			}

			// Act:
			auto keyPairsWrapper = GenerateKeyPairsWrapper();
			context.CustomPrioritizationMap.emplace(keyPairsWrapper.SigningPublicKey, indexToPriorityMap(8));
			auto result = AddAccount(context, std::move(keyPairsWrapper.KeyPairs));

			auto view = accounts.view();
			auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

			// Assert:
			EXPECT_EQ(UnlockedAccountsAddResult::Failure_Server_Limit, result);
			EXPECT_EQ(8u, view.size());
			EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
		}
	}

	TEST(TEST_CLASS, CannotAddMoreThanMaxAccounts_EqualPriority) {
		AssertCannotAddMoreThanMaxAccounts([](auto) { return 7; });
	}

	TEST(TEST_CLASS, CannotAddMoreThanMaxAccounts_PriorityLessThan) {
		AssertCannotAddMoreThanMaxAccounts([](auto i) { return 8 - i; });
	}

	TEST(TEST_CLASS, CannotAddMoreThanMaxAccounts_PriorityGreaterThan) {
		// Arrange:
		constexpr auto Num_Accounts = 8u;
		TestContext context(Num_Accounts);
		const auto& accounts = context.Accounts;

		// -     priorities: 4 3 2 1 0 4 3 2
		// - sorted indexes: 0 2 4 6 7 1 3 5
		std::vector<AccountPublicKeys> expectedPublicKeys;
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto keyPairsWrapper = GenerateKeyPairsWrapper();
			expectedPublicKeys.push_back({ keyPairsWrapper.SigningPublicKey, keyPairsWrapper.VrfPublicKey });
			context.CustomPrioritizationMap.emplace(keyPairsWrapper.SigningPublicKey, 4 - (i % 5));
			AddAccount(context, std::move(keyPairsWrapper.KeyPairs));
		}

		// Act:
		auto keyPairsWrapper = GenerateKeyPairsWrapper();
		context.CustomPrioritizationMap.emplace(keyPairsWrapper.SigningPublicKey, 2);

		expectedPublicKeys[4] = expectedPublicKeys[3]; // lowest is popped
		expectedPublicKeys[3] = { keyPairsWrapper.SigningPublicKey, keyPairsWrapper.VrfPublicKey }; // new key pair is second lowest

		auto result = AddAccount(context, std::move(keyPairsWrapper.KeyPairs));

		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

		// Assert:
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(Num_Accounts, view.size());

		auto expectedIndexes = std::vector<size_t>{ 0, 5, 1, 6, 2, 7, 3, 4 };
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto expectedIndex = expectedIndexes[i];
			EXPECT_EQ(expectedPublicKeys[expectedIndex], actualPublicKeys[i])
					<< "expected index = " << expectedIndex
					<< ", actual index = " << i;
		}
	}

	TEST(TEST_CLASS, RemovedAccountsDoNotCountTowardsLimit) {
		// Arrange:
		auto keyPairsWrapper = GenerateKeyPairsWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		for (auto i = 0u; i < 4; ++i) AddRandomAccount(context);
		AddAccount(context, std::move(keyPairsWrapper.KeyPairs));
		for (auto i = 0u; i < 3; ++i) AddRandomAccount(context);

		// Sanity:
		auto result = AddRandomAccount(context);
		EXPECT_EQ(UnlockedAccountsAddResult::Failure_Server_Limit, result);

		// Act:
		accounts.modifier().remove(keyPairsWrapper.SigningPublicKey);
		result = AddRandomAccount(context);

		// Assert:
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(8u, accounts.view().size());
	}

	// endregion

	// region removeIf

	TEST(TEST_CLASS, RemoveIfRemovesAllAccountsWhenPredicateReturnsTrueForAllAccounts) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;
		for (auto i = 0u; i < 3; ++i)
			AddRandomAccount(context);

		// Sanity:
		EXPECT_EQ(3u, accounts.view().size());

		// Act:
		accounts.modifier().removeIf([](const auto&) { return true; });

		// Assert:
		EXPECT_EQ(0u, accounts.view().size());
	}

	TEST(TEST_CLASS, RemoveIfRemovesNoAccountsWhenPredicateReturnsFalseForAllAccounts) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;
		for (auto i = 0u; i < 3; ++i)
			AddRandomAccount(context);

		// Sanity:
		EXPECT_EQ(3u, accounts.view().size());

		// Act:
		accounts.modifier().removeIf([](const auto&) { return false; });

		// Assert:
		EXPECT_EQ(3u, accounts.view().size());
	}

	TEST(TEST_CLASS, RemoveIfOnlyRemovesAccountsForWhichPredicateReturnsTrue) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;
		for (auto i = 0u; i < 5; ++i)
			AddRandomAccount(context);

		// Sanity:
		EXPECT_EQ(5u, accounts.view().size());

		// Act:
		auto i = 0u;
		std::vector<Key> removedKeys;
		accounts.modifier().removeIf([&](const auto& key) {
			if (0 == i++ % 2)
				return false;

			removedKeys.push_back(key);
			return true;
		});

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(3u, view.size());
		EXPECT_EQ(2u, removedKeys.size());
		for (const auto& key : removedKeys)
			EXPECT_FALSE(view.contains(key));
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<UnlockedAccounts>(7, [](const auto&) { return 0; });
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
