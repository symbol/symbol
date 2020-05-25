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

#include "catapult/state/AccountKeys.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountKeysTests

#define KEY_ACCESSOR_TEST(NAME) TEST(TEST_CLASS, KeyAccessor_##NAME)

	// region KeyAccessor - construction + assignment

	namespace {
		template<typename TAccountPublicKey>
		using KeyAccessor = AccountKeys::KeyAccessor<TAccountPublicKey>;

		template<typename TAccountPublicKey>
		void AssertUnset(const KeyAccessor<TAccountPublicKey>& keyAccessor) {
			EXPECT_FALSE(!!keyAccessor);
			EXPECT_EQ(TAccountPublicKey(), keyAccessor.get());
		}

		template<typename TAccountPublicKey>
		void AssertSet(const KeyAccessor<TAccountPublicKey>& keyAccessor, const TAccountPublicKey& expectedKey) {
			EXPECT_TRUE(!!keyAccessor);
			EXPECT_EQ(expectedKey, keyAccessor.get());
		}

		void AssertCopied(const KeyAccessor<Key>& keyAccessor, const KeyAccessor<Key>& keyAccessorCopy) {
			// Assert: the original values are copied into the copy
			EXPECT_TRUE(!!keyAccessor);
			EXPECT_EQ(Key{ { 0x44 } }, keyAccessor.get());

			EXPECT_TRUE(!!keyAccessorCopy);
			EXPECT_EQ(Key{ { 0x44 } }, keyAccessorCopy.get());
		}

		void AssertDeepCopied(const KeyAccessor<Key>& keyAccessor, const KeyAccessor<Key>& keyAccessorCopy) {
			// Assert: the copy is detached from the original
			EXPECT_TRUE(!!keyAccessor);
			EXPECT_EQ(Key{ { 0x44 } }, keyAccessor.get());

			EXPECT_TRUE(!!keyAccessorCopy);
			EXPECT_EQ(Key{ { 0x32 } }, keyAccessorCopy.get());
		}

		void AssertMoved(const KeyAccessor<Key>& keyAccessor, const KeyAccessor<Key>& keyAccessorMoved) {
			// Assert: the original values are moved into the copy
			EXPECT_FALSE(!!keyAccessor);
			EXPECT_EQ(Key(), keyAccessor.get());

			EXPECT_TRUE(!!keyAccessorMoved);
			EXPECT_EQ(Key{ { 0x44 } }, keyAccessorMoved.get());
		}
	}

	KEY_ACCESSOR_TEST(CanCreateEmpty) {
		// Act:
		KeyAccessor<Key> keyAccessor;

		// Assert:
		AssertUnset(keyAccessor);
	}

	KEY_ACCESSOR_TEST(CanCopyConstruct) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set({ { 0x44 } });

		// Act:
		KeyAccessor<Key> keyAccessorCopy(keyAccessor);

		// Assert:
		AssertCopied(keyAccessor, keyAccessorCopy);

		// Act: modify to check deep copy
		keyAccessorCopy.unset();
		keyAccessorCopy.set({ { 0x32 } });

		// Assert:
		AssertDeepCopied(keyAccessor, keyAccessorCopy);
	}

	KEY_ACCESSOR_TEST(CanMoveConstruct) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set({ { 0x44 } });

		// Act:
		KeyAccessor<Key> keyAccessorMoved(std::move(keyAccessor));

		// Assert:
		AssertMoved(keyAccessor, keyAccessorMoved);
	}

	KEY_ACCESSOR_TEST(CanCopyAssign) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set({ { 0x44 } });

		// Act:
		KeyAccessor<Key> keyAccessorCopy;
		const auto& assignResult = keyAccessorCopy = keyAccessor;

		// Assert:
		EXPECT_EQ(&keyAccessorCopy, &assignResult);
		AssertCopied(keyAccessor, keyAccessorCopy);

		// Act: modify to check deep copy
		keyAccessorCopy.unset();
		keyAccessorCopy.set({ { 0x32 } });

		// Assert:
		AssertDeepCopied(keyAccessor, keyAccessorCopy);
	}

	KEY_ACCESSOR_TEST(CanMoveAssign) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set({ { 0x44 } });

		// Act:
		KeyAccessor<Key> keyAccessorMoved;
		const auto& assignResult = keyAccessorMoved = std::move(keyAccessor);

		// Assert:
		EXPECT_EQ(&keyAccessorMoved, &assignResult);
		AssertMoved(keyAccessor, keyAccessorMoved);
	}

	KEY_ACCESSOR_TEST(CanCopyAssignWhenSourceIsEmpty) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;

		KeyAccessor<Key> keyAccessorCopy;
		keyAccessorCopy.set({ { 0x32 } });

		// Act:
		keyAccessorCopy = keyAccessor;

		// Assert:
		AssertUnset(keyAccessorCopy);
		AssertUnset(keyAccessor);
	}

	KEY_ACCESSOR_TEST(CanMoveAssignWhenSourceIsEmpty) {
		// Arrange:
		KeyAccessor<Key> keyAccessorMoved;
		keyAccessorMoved.set({ { 0x32 } });

		// Act:
		keyAccessorMoved = KeyAccessor<Key>();

		// Assert:
		AssertUnset(keyAccessorMoved);
	}

	// endregion

	// region KeyAccessor - bool / get

	KEY_ACCESSOR_TEST(BoolOperatorReturnsFalseWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set(Key());

		// Act + Assert
		EXPECT_FALSE(!!keyAccessor);
	}

	KEY_ACCESSOR_TEST(GetReturnsZeroKeyWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set(Key());

		// Act + Assert
		EXPECT_EQ(Key(), keyAccessor.get());
	}

	// endregion

	// region set / unset

	KEY_ACCESSOR_TEST(CanSetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		KeyAccessor<Key> keyAccessor;

		// Act:
		keyAccessor.set(key);

		// Assert:
		AssertSet(keyAccessor, key);
	}

	KEY_ACCESSOR_TEST(CannotResetKeyWithValue) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set(key);

		// Act + Assert:
		EXPECT_THROW(keyAccessor.set(key), catapult_invalid_argument);
	}

	KEY_ACCESSOR_TEST(CanUnsetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set(key);

		// Act:
		keyAccessor.unset();

		// Assert:
		AssertUnset(keyAccessor);
	}

	KEY_ACCESSOR_TEST(CanUnsetKeyWhenEmpty) {
		// Arrange:
		KeyAccessor<Key> keyAccessor;
		keyAccessor.set(test::GenerateRandomByteArray<Key>());
		keyAccessor.unset();

		// Act:
		keyAccessor.unset();

		// Assert:
		AssertUnset(keyAccessor);
	}

	KEY_ACCESSOR_TEST(CanSetKeyAfterUnset) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		KeyAccessor<Key> keyAccessor;

		keyAccessor.set(test::GenerateRandomByteArray<Key>());
		keyAccessor.unset();

		// Act:
		keyAccessor.set(key);

		// Assert:
		AssertSet(keyAccessor, key);
	}

	// endregion

	// region AccountKeys

	TEST(TEST_CLASS, CanCreateEmptyContainer) {
		// Act:
		AccountKeys keys;

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::Unset, keys.mask());

		// - all keys are unset
		EXPECT_FALSE(!!keys.linkedPublicKey());
		EXPECT_FALSE(!!keys.vrfPublicKey());
		EXPECT_FALSE(!!keys.votingPublicKey());
		EXPECT_FALSE(!!keys.nodePublicKey());

		// - const and non-const accessors reference same objects
		EXPECT_EQ(&keys.linkedPublicKey(), &const_cast<const AccountKeys&>(keys).linkedPublicKey());
		EXPECT_EQ(&keys.vrfPublicKey(), &const_cast<const AccountKeys&>(keys).vrfPublicKey());
		EXPECT_EQ(&keys.votingPublicKey(), &const_cast<const AccountKeys&>(keys).votingPublicKey());
		EXPECT_EQ(&keys.nodePublicKey(), &const_cast<const AccountKeys&>(keys).nodePublicKey());
	}

	TEST(TEST_CLASS, CanDeepCopy) {
		// Arrange: sanity check that copied keys are unlinked
		auto key1 = test::GenerateRandomByteArray<Key>();
		auto key2 = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;
		keys.linkedPublicKey().set(key1);

		// Act:
		AccountKeys keysCopy;
		keysCopy = keys;
		keysCopy.linkedPublicKey().unset();
		keysCopy.linkedPublicKey().set(key2);

		// Assert:
		EXPECT_EQ(key1, keys.linkedPublicKey().get());
		EXPECT_EQ(key2, keysCopy.linkedPublicKey().get());
	}

	TEST(TEST_CLASS, CanSetLinkedPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		// Act:
		keys.linkedPublicKey().set(key);

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::Linked, keys.mask());

		// - one key is set
		EXPECT_TRUE(!!keys.linkedPublicKey());
		EXPECT_FALSE(!!keys.vrfPublicKey());
		EXPECT_FALSE(!!keys.votingPublicKey());
		EXPECT_FALSE(!!keys.nodePublicKey());

		EXPECT_EQ(key, keys.linkedPublicKey().get());
	}

	TEST(TEST_CLASS, CanSetVrfPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		// Act:
		keys.vrfPublicKey().set(key);

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::VRF, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linkedPublicKey());
		EXPECT_TRUE(!!keys.vrfPublicKey());
		EXPECT_FALSE(!!keys.votingPublicKey());
		EXPECT_FALSE(!!keys.nodePublicKey());

		EXPECT_EQ(key, keys.vrfPublicKey().get());
	}

	TEST(TEST_CLASS, CanSetVotingPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<VotingKey>();
		AccountKeys keys;

		// Act:
		keys.votingPublicKey().set(key);

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::Voting, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linkedPublicKey());
		EXPECT_FALSE(!!keys.vrfPublicKey());
		EXPECT_TRUE(!!keys.votingPublicKey());
		EXPECT_FALSE(!!keys.nodePublicKey());

		EXPECT_EQ(key, keys.votingPublicKey().get());
	}

	TEST(TEST_CLASS, CanSetNodePublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		// Act:
		keys.nodePublicKey().set(key);

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::Node, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linkedPublicKey());
		EXPECT_FALSE(!!keys.vrfPublicKey());
		EXPECT_FALSE(!!keys.votingPublicKey());
		EXPECT_TRUE(!!keys.nodePublicKey());

		EXPECT_EQ(key, keys.nodePublicKey().get());
	}

	TEST(TEST_CLASS, CanSetAllKeys) {
		// Arrange:
		auto linkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto vrfPublicKey = test::GenerateRandomByteArray<Key>();
		auto votingPublicKey = test::GenerateRandomByteArray<VotingKey>();
		auto nodePublicKey = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		// Act:
		keys.linkedPublicKey().set(linkedPublicKey);
		keys.vrfPublicKey().set(vrfPublicKey);
		keys.votingPublicKey().set(votingPublicKey);
		keys.nodePublicKey().set(nodePublicKey);

		// Assert:
		EXPECT_EQ(AccountKeys::KeyType::All, keys.mask());

		// - one key is set
		EXPECT_TRUE(!!keys.linkedPublicKey());
		EXPECT_TRUE(!!keys.vrfPublicKey());
		EXPECT_TRUE(!!keys.votingPublicKey());
		EXPECT_TRUE(!!keys.nodePublicKey());

		EXPECT_EQ(linkedPublicKey, keys.linkedPublicKey().get());
		EXPECT_EQ(vrfPublicKey, keys.vrfPublicKey().get());
		EXPECT_EQ(votingPublicKey, keys.votingPublicKey().get());
		EXPECT_EQ(nodePublicKey, keys.nodePublicKey().get());
	}

	// endregion
}}
