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

#include "catapult/state/AccountPublicKeys.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountPublicKeysTests

#define PUBLIC_KEY_ACCESSOR_TEST(NAME) TEST(TEST_CLASS, PublicKeyAccessor_##NAME)

	// region PublicKeyAccessor - construction + assignment

	namespace {
		template<typename TAccountPublicKey>
		using PublicKeyAccessor = AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>;

		template<typename TAccountPublicKey>
		void AssertUnset(const PublicKeyAccessor<TAccountPublicKey>& accessor) {
			EXPECT_FALSE(!!accessor);
			EXPECT_EQ(TAccountPublicKey(), accessor.get());
		}

		template<typename TAccountPublicKey>
		void AssertSet(const PublicKeyAccessor<TAccountPublicKey>& accessor, const TAccountPublicKey& expectedKey) {
			EXPECT_TRUE(!!accessor);
			EXPECT_EQ(expectedKey, accessor.get());
		}

		void AssertCopied(const PublicKeyAccessor<Key>& accessor, const PublicKeyAccessor<Key>& accessorCopy) {
			// Assert: the original values are copied into the copy
			EXPECT_TRUE(!!accessor);
			EXPECT_EQ(Key{ { 0x44 } }, accessor.get());

			EXPECT_TRUE(!!accessorCopy);
			EXPECT_EQ(Key{ { 0x44 } }, accessorCopy.get());
		}

		void AssertDeepCopied(const PublicKeyAccessor<Key>& accessor, const PublicKeyAccessor<Key>& accessorCopy) {
			// Assert: the copy is detached from the original
			EXPECT_TRUE(!!accessor);
			EXPECT_EQ(Key{ { 0x44 } }, accessor.get());

			EXPECT_TRUE(!!accessorCopy);
			EXPECT_EQ(Key{ { 0x32 } }, accessorCopy.get());
		}

		void AssertMoved(const PublicKeyAccessor<Key>& accessor, const PublicKeyAccessor<Key>& accessorMoved) {
			// Assert: the original values are moved into the copy
			EXPECT_FALSE(!!accessor);
			EXPECT_EQ(Key(), accessor.get());

			EXPECT_TRUE(!!accessorMoved);
			EXPECT_EQ(Key{ { 0x44 } }, accessorMoved.get());
		}
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCreateEmpty) {
		// Act:
		PublicKeyAccessor<Key> accessor;

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyConstruct) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor<Key> accessorCopy(accessor);

		// Assert:
		AssertCopied(accessor, accessorCopy);

		// Act: modify to check deep copy
		accessorCopy.unset();
		accessorCopy.set({ { 0x32 } });

		// Assert:
		AssertDeepCopied(accessor, accessorCopy);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanMoveConstruct) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor<Key> accessorMoved(std::move(accessor));

		// Assert:
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyAssign) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor<Key> accessorCopy;
		const auto& assignResult = accessorCopy = accessor;

		// Assert:
		EXPECT_EQ(&accessorCopy, &assignResult);
		AssertCopied(accessor, accessorCopy);

		// Act: modify to check deep copy
		accessorCopy.unset();
		accessorCopy.set({ { 0x32 } });

		// Assert:
		AssertDeepCopied(accessor, accessorCopy);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanMoveAssign) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor<Key> accessorMoved;
		const auto& assignResult = accessorMoved = std::move(accessor);

		// Assert:
		EXPECT_EQ(&accessorMoved, &assignResult);
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;

		PublicKeyAccessor<Key> accessorCopy;
		accessorCopy.set({ { 0x32 } });

		// Act:
		accessorCopy = accessor;

		// Assert:
		AssertUnset(accessorCopy);
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanMoveAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeyAccessor<Key> accessorMoved;
		accessorMoved.set({ { 0x32 } });

		// Act:
		accessorMoved = PublicKeyAccessor<Key>();

		// Assert:
		AssertUnset(accessorMoved);
	}

	// endregion

	// region PublicKeyAccessor - bool / get

	PUBLIC_KEY_ACCESSOR_TEST(BoolOperatorReturnsFalseWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set(Key());

		// Act + Assert
		EXPECT_FALSE(!!accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(GetReturnsZeroKeyWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set(Key());

		// Act + Assert
		EXPECT_EQ(Key(), accessor.get());
	}

	// endregion

	// region set / unset

	PUBLIC_KEY_ACCESSOR_TEST(CanSetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor<Key> accessor;

		// Act:
		accessor.set(key);

		// Assert:
		AssertSet(accessor, key);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CannotResetKeyWithValue) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor<Key> accessor;
		accessor.set(key);

		// Act + Assert:
		EXPECT_THROW(accessor.set(key), catapult_invalid_argument);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanUnsetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor<Key> accessor;
		accessor.set(key);

		// Act:
		accessor.unset();

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanUnsetKeyWhenEmpty) {
		// Arrange:
		PublicKeyAccessor<Key> accessor;
		accessor.set(test::GenerateRandomByteArray<Key>());
		accessor.unset();

		// Act:
		accessor.unset();

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanSetKeyAfterUnset) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor<Key> accessor;

		accessor.set(test::GenerateRandomByteArray<Key>());
		accessor.unset();

		// Act:
		accessor.set(key);

		// Assert:
		AssertSet(accessor, key);
	}

	// endregion

	// region AccountPublicKeys

	TEST(TEST_CLASS, CanCreateEmptyContainer) {
		// Act:
		AccountPublicKeys keys;

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::Unset, keys.mask());

		// - all keys are unset
		EXPECT_FALSE(!!keys.linked());
		EXPECT_FALSE(!!keys.node());
		EXPECT_FALSE(!!keys.vrf());
		EXPECT_FALSE(!!keys.voting());

		// - const and non-const accessors reference same objects
		EXPECT_EQ(&keys.linked(), &const_cast<const AccountPublicKeys&>(keys).linked());
		EXPECT_EQ(&keys.node(), &const_cast<const AccountPublicKeys&>(keys).node());
		EXPECT_EQ(&keys.vrf(), &const_cast<const AccountPublicKeys&>(keys).vrf());
		EXPECT_EQ(&keys.voting(), &const_cast<const AccountPublicKeys&>(keys).voting());
	}

	TEST(TEST_CLASS, CanDeepCopy) {
		// Arrange: sanity check that copied keys are unlinked
		auto key1 = test::GenerateRandomByteArray<Key>();
		auto key2 = test::GenerateRandomByteArray<Key>();
		AccountPublicKeys keys;
		keys.linked().set(key1);

		// Act:
		AccountPublicKeys keysCopy;
		keysCopy = keys;
		keysCopy.linked().unset();
		keysCopy.linked().set(key2);

		// Assert:
		EXPECT_EQ(key1, keys.linked().get());
		EXPECT_EQ(key2, keysCopy.linked().get());
	}

	TEST(TEST_CLASS, CanSetLinkedPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountPublicKeys keys;

		// Act:
		keys.linked().set(key);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::Linked, keys.mask());

		// - one key is set
		EXPECT_TRUE(!!keys.linked());
		EXPECT_FALSE(!!keys.node());
		EXPECT_FALSE(!!keys.vrf());
		EXPECT_FALSE(!!keys.voting());

		EXPECT_EQ(key, keys.linked().get());
	}

	TEST(TEST_CLASS, CanSetNodePublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountPublicKeys keys;

		// Act:
		keys.node().set(key);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::Node, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linked());
		EXPECT_TRUE(!!keys.node());
		EXPECT_FALSE(!!keys.vrf());
		EXPECT_FALSE(!!keys.voting());

		EXPECT_EQ(key, keys.node().get());
	}

	TEST(TEST_CLASS, CanSetVrfPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountPublicKeys keys;

		// Act:
		keys.vrf().set(key);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::VRF, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linked());
		EXPECT_FALSE(!!keys.node());
		EXPECT_TRUE(!!keys.vrf());
		EXPECT_FALSE(!!keys.voting());

		EXPECT_EQ(key, keys.vrf().get());
	}

	TEST(TEST_CLASS, CanSetVotingPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomPackedStruct<PinnedVotingKey>();
		AccountPublicKeys keys;

		// Act:
		keys.voting().set(key);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::Voting, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linked());
		EXPECT_FALSE(!!keys.node());
		EXPECT_FALSE(!!keys.vrf());
		EXPECT_TRUE(!!keys.voting());

		EXPECT_EQ(key, keys.voting().get());
	}

	TEST(TEST_CLASS, CanSetAllKeys) {
		// Arrange:
		auto linkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto nodePublicKey = test::GenerateRandomByteArray<Key>();
		auto vrfPublicKey = test::GenerateRandomByteArray<Key>();
		auto votingPublicKey = test::GenerateRandomPackedStruct<PinnedVotingKey>();
		AccountPublicKeys keys;

		// Act:
		keys.linked().set(linkedPublicKey);
		keys.node().set(nodePublicKey);
		keys.vrf().set(vrfPublicKey);
		keys.voting().set(votingPublicKey);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::All, keys.mask());

		// - one key is set
		EXPECT_TRUE(!!keys.linked());
		EXPECT_TRUE(!!keys.node());
		EXPECT_TRUE(!!keys.vrf());
		EXPECT_TRUE(!!keys.voting());

		EXPECT_EQ(linkedPublicKey, keys.linked().get());
		EXPECT_EQ(nodePublicKey, keys.node().get());
		EXPECT_EQ(vrfPublicKey, keys.vrf().get());
		EXPECT_EQ(votingPublicKey, keys.voting().get());
	}

	// endregion
}}
