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
		struct KeyPairWrapper {
		public:
			crypto::KeyPair KeyPair;
			Key PublicKey;

		public:
			KeyPairWrapper() : KeyPair(test::GenerateKeyPair()), PublicKey(KeyPair.publicKey())
			{}
		};

		KeyPairWrapper GenerateKeyPairWrapper() {
			return KeyPairWrapper();
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

		UnlockedAccountsAddResult AddAccount(TestContext& context, crypto::KeyPair&& keyPair) {
			return context.Accounts.modifier().add(std::move(keyPair));
		}

		UnlockedAccountsAddResult AddRandomAccount(TestContext& context) {
			return AddAccount(context, test::GenerateKeyPair());
		}
	}

	// endregion

	// region basic ctor, add, remove

	TEST(TEST_CLASS, InitiallyContainerIsEmpty) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		// Assert:
		EXPECT_EQ(0u, accounts.view().size());
	}

	TEST(TEST_CLASS, CanAddHarvestingEligibleAccount) {
		// Arrange:
		auto keyPairWrapper = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result = accounts.modifier().add(std::move(keyPairWrapper.KeyPair));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairWrapper.PublicKey));
	}

	TEST(TEST_CLASS, AdditionOfAlreadyAddedAccountHasNoEffect) {
		// Arrange:
		constexpr auto Private_Key_String = "3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB";
		auto keyPair1 = crypto::KeyPair::FromString(Private_Key_String);
		auto keyPair2 = crypto::KeyPair::FromString(Private_Key_String);
		auto publicKey = keyPair1.publicKey();

		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result1 = accounts.modifier().add(std::move(keyPair1));
		auto result2 = accounts.modifier().add(std::move(keyPair2));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result1);
		EXPECT_EQ(UnlockedAccountsAddResult::Success_Redundant, result2);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(publicKey));
	}

	TEST(TEST_CLASS, CanAddMultipleAccountsToContainer) {
		// Arrange:
		auto keyPairWrapper1 = GenerateKeyPairWrapper();
		auto keyPairWrapper2 = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		{
			auto modifier = accounts.modifier();
			modifier.add(std::move(keyPairWrapper1.KeyPair));
			modifier.add(std::move(keyPairWrapper2.KeyPair));
		}

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(2u, view.size());
		EXPECT_TRUE(view.contains(keyPairWrapper1.PublicKey));
		EXPECT_TRUE(view.contains(keyPairWrapper2.PublicKey));
	}

	TEST(TEST_CLASS, CanRemoveAccountFromContainer) {
		// Arrange:
		auto keyPairWrapper1 = GenerateKeyPairWrapper();
		auto keyPairWrapper2 = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		{
			auto modifier = accounts.modifier();
			modifier.add(std::move(keyPairWrapper1.KeyPair));
			modifier.add(std::move(keyPairWrapper2.KeyPair));
		}

		// Sanity:
		EXPECT_EQ(2u, accounts.view().size());

		// Act:
		auto removeResult = accounts.modifier().remove(keyPairWrapper1.PublicKey);

		// Assert:
		EXPECT_TRUE(removeResult);

		auto view = accounts.view();
		EXPECT_EQ(1u, view.size());
		EXPECT_FALSE(view.contains(keyPairWrapper1.PublicKey));
		EXPECT_TRUE(view.contains(keyPairWrapper2.PublicKey));
	}

	TEST(TEST_CLASS, RemovalOfAccountNotInContainerHasNoEffect) {
		// Arrange:
		auto keyPairWrapper = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result = accounts.modifier().add(std::move(keyPairWrapper.KeyPair));
		auto removeResult = accounts.modifier().remove(test::GenerateKeyPair().publicKey());

		// Assert:
		EXPECT_FALSE(removeResult);

		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success_New, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairWrapper.PublicKey));
	}

	// endregion

	// region forEach iteration

	namespace {
		std::vector<Key> AddAccounts(TestContext& context, size_t numAccounts) {
			std::vector<Key> publicKeys;
			for (auto i = 0u; i < numAccounts; ++i) {
				auto keyPair = test::GenerateKeyPair();
				publicKeys.push_back(keyPair.publicKey());
				AddAccount(context, std::move(keyPair));
			}

			return publicKeys;
		}

		std::vector<Key> ExtractAllPublicKeysOrdered(
				const UnlockedAccountsView& view,
				size_t maxPublicKeys = std::numeric_limits<size_t>::max()) {
			std::vector<Key> publicKeys;
			view.forEach([maxPublicKeys, &publicKeys](const auto& keyPair) {
				publicKeys.push_back(keyPair.publicKey());
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
			auto keyPair = test::GenerateKeyPair();
			expectedPublicKeys.push_back(keyPair.publicKey());
			context.CustomPrioritizationMap.emplace(keyPair.publicKey(), 2 - (i % 3));
			AddAccount(context, std::move(keyPair));
		}

		// Act:
		auto view = accounts.view();
		auto actualPublicKeys = ExtractAllPublicKeysOrdered(view);

		// Assert:
		EXPECT_EQ(Num_Accounts, view.size());
		EXPECT_EQ(Num_Accounts, actualPublicKeys.size());
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto expectedIndex = (i / 4) + 3 * (i % 4);
			EXPECT_EQ(expectedPublicKeys[expectedIndex], actualPublicKeys[i])
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
			modifier.remove(*++expectedPublicKeys.cbegin());
			modifier.remove(*--expectedPublicKeys.cend());
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

			std::vector<Key> expectedPublicKeys;
			for (auto i = 0u; i < 8; ++i) {
				auto keyPair = test::GenerateKeyPair();
				expectedPublicKeys.push_back(keyPair.publicKey());
				context.CustomPrioritizationMap.emplace(keyPair.publicKey(), indexToPriorityMap(i));
				AddAccount(context, std::move(keyPair));
			}

			// Act:
			auto keyPair = test::GenerateKeyPair();
			context.CustomPrioritizationMap.emplace(keyPair.publicKey(), indexToPriorityMap(8));
			auto result = AddAccount(context, std::move(keyPair));

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
		std::vector<Key> expectedPublicKeys;
		for (auto i = 0u; i < Num_Accounts; ++i) {
			auto keyPair = test::GenerateKeyPair();
			expectedPublicKeys.push_back(keyPair.publicKey());
			context.CustomPrioritizationMap.emplace(keyPair.publicKey(), 4 - (i % 5));
			AddAccount(context, std::move(keyPair));
		}

		// Act:
		auto keyPair = test::GenerateKeyPair();
		context.CustomPrioritizationMap.emplace(keyPair.publicKey(), 2);

		expectedPublicKeys[4] = expectedPublicKeys[3]; // lowest is popped
		expectedPublicKeys[3] = keyPair.publicKey(); // new key pair is second lowest

		auto result = AddAccount(context, std::move(keyPair));

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
		auto keyPairWrapper = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		for (auto i = 0u; i < 4; ++i) AddRandomAccount(context);
		AddAccount(context, std::move(keyPairWrapper.KeyPair));
		for (auto i = 0u; i < 3; ++i) AddRandomAccount(context);

		// Sanity:
		auto result = AddRandomAccount(context);
		EXPECT_EQ(UnlockedAccountsAddResult::Failure_Server_Limit, result);

		// Act:
		accounts.modifier().remove(keyPairWrapper.PublicKey);
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
