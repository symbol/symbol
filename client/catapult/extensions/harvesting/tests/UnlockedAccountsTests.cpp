#include "harvesting/src/UnlockedAccounts.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/KeyPairTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedAccountsTests

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
			explicit TestContext(size_t maxSize) : Accounts(maxSize)
			{}

		public:
			UnlockedAccounts Accounts;
		};
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
		auto keyPairWrapper = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		auto result = accounts.modifier().add(std::move(keyPairWrapper.KeyPair));

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairWrapper.PublicKey));
	}

	TEST(TEST_CLASS, AdditionOfAlreadyAddedAccountHasNoEffect) {
		// Arrange:
		constexpr auto Private_Key_String = "3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb";
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
		EXPECT_EQ(UnlockedAccountsAddResult::Success, result1);
		EXPECT_EQ(UnlockedAccountsAddResult::Success, result2);
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
		accounts.modifier().remove(keyPairWrapper1.PublicKey);

		// Assert:
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
		accounts.modifier().remove(test::GenerateKeyPair().publicKey());

		// Assert:
		auto view = accounts.view();
		EXPECT_EQ(UnlockedAccountsAddResult::Success, result);
		EXPECT_EQ(1u, view.size());
		EXPECT_TRUE(view.contains(keyPairWrapper.PublicKey));
	}

	namespace {
		UnlockedAccountsAddResult AddAccount(TestContext& context, crypto::KeyPair&& keyPair) {
			return context.Accounts.modifier().add(std::move(keyPair));
		}
	}

	TEST(TEST_CLASS, CanIterateOverAllAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		std::set<Key> expectedPublicKeys;
		for (auto i = 0u; i < 4u; ++i) {
			auto keyPair = test::GenerateKeyPair();
			expectedPublicKeys.insert(keyPair.publicKey());
			AddAccount(context, std::move(keyPair));
		}

		// Act:
		auto view = accounts.view();
		std::set<Key> actualPublicKeys;
		for (const auto& keyPair : view)
			actualPublicKeys.insert(keyPair.publicKey());

		// Assert:
		EXPECT_EQ(4u, view.size());
		EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
	}

	TEST(TEST_CLASS, RemovedAccountsAreNotIterated) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;

		std::set<Key> expectedPublicKeys;
		for (auto i = 0u; i < 4u; ++i) {
			auto keyPair = test::GenerateKeyPair();
			expectedPublicKeys.insert(keyPair.publicKey());
			AddAccount(context, std::move(keyPair));
		}

		{
			auto modifier = accounts.modifier();
			modifier.remove(*++expectedPublicKeys.cbegin());
			modifier.remove(*--expectedPublicKeys.cend());
		}

		expectedPublicKeys.erase(++expectedPublicKeys.begin());
		expectedPublicKeys.erase(--expectedPublicKeys.cend());

		// Act:
		auto view = accounts.view();
		std::set<Key> actualPublicKeys;
		for (const auto& keyPair : view)
			actualPublicKeys.insert(keyPair.publicKey());

		// Assert:
		EXPECT_EQ(2u, view.size());
		EXPECT_EQ(expectedPublicKeys, actualPublicKeys);
	}

	namespace {
		UnlockedAccountsAddResult AddRandomAccount(TestContext& context) {
			return AddAccount(context, test::GenerateKeyPair());
		}
	}

	TEST(TEST_CLASS, CanAddMaxAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		// Act:
		for (auto i = 0u; i < 8u; ++i)
			AddRandomAccount(context);

		// Assert:
		EXPECT_EQ(8u, accounts.view().size());
	}

	TEST(TEST_CLASS, CannotAddMoreThanMaxAccounts) {
		// Arrange:
		TestContext context(8);
		const auto& accounts = context.Accounts;

		for (auto i = 0u; i < 8u; ++i)
			AddRandomAccount(context);

		// Act:
		auto result = AddRandomAccount(context);

		// Assert:
		EXPECT_EQ(UnlockedAccountsAddResult::Failure_Server_Limit, result);
		EXPECT_EQ(8u, accounts.view().size());
	}

	TEST(TEST_CLASS, RemovedAccountsDoNotCountTowardsLimit) {
		// Arrange:
		auto keyPairWrapper = GenerateKeyPairWrapper();
		TestContext context(8);
		auto& accounts = context.Accounts;

		// Act:
		for (auto i = 0u; i < 4u; ++i) AddRandomAccount(context);
		AddAccount(context, std::move(keyPairWrapper.KeyPair));
		for (auto i = 0u; i < 3u; ++i) AddRandomAccount(context);

		// Sanity:
		auto result = AddRandomAccount(context);
		EXPECT_EQ(UnlockedAccountsAddResult::Failure_Server_Limit, result);

		// Act:
		accounts.modifier().remove(keyPairWrapper.PublicKey);
		result = AddRandomAccount(context);

		// Assert:
		EXPECT_EQ(UnlockedAccountsAddResult::Success, result);
		EXPECT_EQ(8u, accounts.view().size());
	}

	// region removeIf

	TEST(TEST_CLASS, RemoveIfRemovesAllAccountsIfPredicateReturnsTrueForAllAccounts) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;
		for (auto i = 0u; i < 3u; ++i)
			AddRandomAccount(context);

		// Sanity:
		EXPECT_EQ(3u, accounts.view().size());

		// Act:
		accounts.modifier().removeIf([](const auto&) { return true; });

		// Assert:
		EXPECT_EQ(0u, accounts.view().size());
	}

	TEST(TEST_CLASS, RemoveIfRemovesNoAccountsIfPredicateReturnsFalseForAllAccounts) {
		// Arrange:
		TestContext context(8);
		auto& accounts = context.Accounts;
		for (auto i = 0u; i < 3u; ++i)
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
		for (auto i = 0u; i < 5u; ++i)
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
			return std::make_unique<UnlockedAccounts>(7);
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
