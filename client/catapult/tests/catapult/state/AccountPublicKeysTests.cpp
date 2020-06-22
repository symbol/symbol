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
#define PUBLIC_KEYS_ACCESSOR_TEST(NAME) TEST(TEST_CLASS, PublicKeysAccessor_##NAME)

	// region PublicKeyAccessor - construction + assignment

	namespace {
		using PublicKeyAccessor = AccountPublicKeys::PublicKeyAccessor<Key>;

		void AssertUnset(const PublicKeyAccessor& accessor) {
			EXPECT_FALSE(!!accessor);
			EXPECT_EQ(Key(), accessor.get());
		}

		void AssertSet(const PublicKeyAccessor& accessor, const Key& expectedKey) {
			EXPECT_TRUE(!!accessor);
			EXPECT_EQ(expectedKey, accessor.get());
		}

		void AssertCopied(const PublicKeyAccessor& accessor, const PublicKeyAccessor& accessorCopy) {
			// Assert: the original values are copied into the copy
			AssertSet(accessor, { { 0x44 } });

			AssertSet(accessorCopy, { { 0x44 } });
		}

		void AssertDeepCopied(const PublicKeyAccessor& accessor, const PublicKeyAccessor& accessorCopy) {
			// Assert: the copy is detached from the original
			AssertSet(accessor, { { 0x44 } });

			AssertSet(accessorCopy, { { 0x32 } });
		}

		void AssertMoved(const PublicKeyAccessor& accessor, const PublicKeyAccessor& accessorMoved) {
			// Assert: the original values are moved into the copy
			AssertUnset(accessor);

			AssertSet(accessorMoved, { { 0x44 } });
		}
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCreateEmpty) {
		// Act:
		PublicKeyAccessor accessor;

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyConstruct) {
		// Arrange:
		PublicKeyAccessor accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor accessorCopy(accessor);

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
		PublicKeyAccessor accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor accessorMoved(std::move(accessor));

		// Assert:
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyAssign) {
		// Arrange:
		PublicKeyAccessor accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor accessorCopy;
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
		PublicKeyAccessor accessor;
		accessor.set({ { 0x44 } });

		// Act:
		PublicKeyAccessor accessorMoved;
		const auto& assignResult = accessorMoved = std::move(accessor);

		// Assert:
		EXPECT_EQ(&accessorMoved, &assignResult);
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanCopyAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeyAccessor accessor;

		PublicKeyAccessor accessorCopy;
		accessorCopy.set({ { 0x32 } });

		// Act:
		accessorCopy = accessor;

		// Assert:
		AssertUnset(accessorCopy);
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanMoveAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeyAccessor accessorMoved;
		accessorMoved.set({ { 0x32 } });

		// Act:
		accessorMoved = PublicKeyAccessor();

		// Assert:
		AssertUnset(accessorMoved);
	}

	// endregion

	// region PublicKeyAccessor - bool / get

	PUBLIC_KEY_ACCESSOR_TEST(BoolOperatorReturnsFalseWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		PublicKeyAccessor accessor;
		accessor.set(Key());

		// Act + Assert:
		EXPECT_FALSE(!!accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(GetReturnsZeroKeyWhenZeroKeyIsExplicitlySet) {
		// Arrange:
		PublicKeyAccessor accessor;
		accessor.set(Key());

		// Act + Assert:
		EXPECT_EQ(Key(), accessor.get());
	}

	// endregion

	// region PublicKeyAccessor - set / unset

	PUBLIC_KEY_ACCESSOR_TEST(CanSetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor accessor;

		// Act:
		accessor.set(key);

		// Assert:
		AssertSet(accessor, key);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CannotResetKeyWithValue) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor accessor;
		accessor.set(key);

		// Act + Assert:
		EXPECT_THROW(accessor.set(key), catapult_invalid_argument);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanUnsetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		PublicKeyAccessor accessor;
		accessor.set(key);

		// Act:
		accessor.unset();

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CanUnsetKeyWhenEmpty) {
		// Arrange:
		PublicKeyAccessor accessor;
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
		PublicKeyAccessor accessor;

		accessor.set(test::GenerateRandomByteArray<Key>());
		accessor.unset();

		// Act:
		accessor.set(key);

		// Assert:
		AssertSet(accessor, key);
	}

	// endregion

	// region PublicKeysAccessor - construction + assignment

	namespace {
		using FP = FinalizationPoint;
		using PublicKeysAccessor = AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>;

		void AssertUnset(const PublicKeysAccessor& accessor) {
			EXPECT_EQ(0u, accessor.size());
		}

		void AssertSet(const PublicKeysAccessor& accessor, const std::vector<model::PinnedVotingKey>& expectedKeys) {
			ASSERT_EQ(expectedKeys.size(), accessor.size());

			for (auto i = 0u; i < expectedKeys.size(); ++i)
				EXPECT_EQ(expectedKeys[i], accessor.get(i)) << "at " << i;
		}

		void AssertCopied(const PublicKeysAccessor& accessor, const PublicKeysAccessor& accessorCopy) {
			// Assert: the original values are copied into the copy
			AssertSet(accessor, {
				{ { { 0x44 } }, FP(100), FP(149) }
			});

			AssertSet(accessorCopy, {
				{ { { 0x44 } }, FP(100), FP(149) }
			});
		}

		void AssertDeepCopied(const PublicKeysAccessor& accessor, const PublicKeysAccessor& accessorCopy) {
			// Assert: the copy is detached from the original
			AssertSet(accessor, {
				{ { { 0x44 } }, FP(100), FP(149) }
			});

			AssertSet(accessorCopy, {
				{ { { 0x44 } }, FP(100), FP(149) },
				{ { { 0x32 } }, FP(200), FP(299) }
			});
		}

		void AssertMoved(const PublicKeysAccessor& accessor, const PublicKeysAccessor& accessorMoved) {
			// Assert: the original values are moved into the copy
			AssertUnset(accessor);

			AssertSet(accessorMoved, {
				{ { { 0x44 } }, FP(100), FP(149) }
			});
		}
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanCreateEmpty) {
		// Act:
		PublicKeysAccessor accessor;

		// Assert:
		AssertUnset(accessor);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanCopyConstruct) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });

		// Act:
		PublicKeysAccessor accessorCopy(accessor);

		// Assert:
		AssertCopied(accessor, accessorCopy);

		// Act: modify to check deep copy
		accessorCopy.add({ { { 0x32 } }, FP(200), FP(299) });

		// Assert:
		AssertDeepCopied(accessor, accessorCopy);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanMoveConstruct) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });

		// Act:
		PublicKeysAccessor accessorMoved(std::move(accessor));

		// Assert:
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanCopyAssign) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });

		// Act:
		PublicKeysAccessor accessorCopy;
		const auto& assignResult = accessorCopy = accessor;

		// Assert:
		EXPECT_EQ(&accessorCopy, &assignResult);
		AssertCopied(accessor, accessorCopy);

		// Act: modify to check deep copy
		accessorCopy.add({ { { 0x32 } }, FP(200), FP(299) });

		// Assert:
		AssertDeepCopied(accessor, accessorCopy);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanMoveAssign) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });

		// Act:
		PublicKeysAccessor accessorMoved;
		const auto& assignResult = accessorMoved = std::move(accessor);

		// Assert:
		EXPECT_EQ(&accessorMoved, &assignResult);
		AssertMoved(accessor, accessorMoved);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanCopyAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeysAccessor accessor;

		PublicKeysAccessor accessorCopy;
		accessorCopy.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		accessorCopy = accessor;

		// Assert:
		AssertUnset(accessorCopy);
		AssertUnset(accessor);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanMoveAssignWhenSourceIsEmpty) {
		// Arrange:
		PublicKeysAccessor accessorMoved;
		accessorMoved.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		accessorMoved = PublicKeysAccessor();

		// Assert:
		AssertUnset(accessorMoved);
	}

	// endregion

	// region PublicKeysAccessor - upperBound

	PUBLIC_KEYS_ACCESSOR_TEST(UpperBoundReturnsZeroWhenNoKeysAreSet) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act + Assert:
		EXPECT_EQ(FP(0), accessor.upperBound());
	}

	PUBLIC_KEYS_ACCESSOR_TEST(UpperBoundReturnsLargestEndPointWhenKeysAreSet) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x11 } }, FP(1), FP(50) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });

		// Act + Assert:
		EXPECT_EQ(FP(180), accessor.upperBound());
	}

	// endregion

	// region PublicKeysAccessor - contains

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsReturnsFalseWhenNoKeysAreSet) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act:
		for (auto value : std::initializer_list<uint64_t>{ 100, 125, 149, 200, 250, 299 }) {
			auto resultPair = accessor.contains(FP(value));

			// Assert:
			EXPECT_FALSE(resultPair.second) << value;
			EXPECT_EQ(std::numeric_limits<size_t>::max(), resultPair.first) << value;
		}
	}

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsReturnsTrueForFinalizationPointsWithAssociatedKey) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		for (auto value : std::initializer_list<uint64_t>{ 100, 125, 149 }) {
			auto resultPair = accessor.contains(FP(value));

			// Assert:
			EXPECT_TRUE(resultPair.second) << value;
			EXPECT_EQ(0u, resultPair.first) << value;
		}

		// Act:
		for (auto value : std::initializer_list<uint64_t>{ 200, 250, 299 }) {
			auto resultPair = accessor.contains(FP(value));

			// Assert:
			EXPECT_TRUE(resultPair.second) << value;
			EXPECT_EQ(1u, resultPair.first) << value;
		}
	}

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsReturnsFalseForFinalizationPointsWithoutAssociatedKey) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		for (auto value : std::initializer_list<uint64_t>{ 1, 99, 150, 175, 199, 300, 500 }) {
			auto resultPair = accessor.contains(FP(value));

			// Assert:
			EXPECT_FALSE(resultPair.second) << value;
			EXPECT_EQ(std::numeric_limits<size_t>::max(), resultPair.first) << value;
		}
	}

	// endregion

	// region PublicKeysAccessor - containsExact

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsExactReturnsFalseWhenNoKeysAreSet) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act + Assert:
		EXPECT_FALSE(accessor.containsExact({ { { 0x44 } }, FP(100), FP(149) }));
		EXPECT_FALSE(accessor.containsExact({ { { 0x98 } }, FP(150), FP(180) }));
		EXPECT_FALSE(accessor.containsExact({ { { 0x32 } }, FP(200), FP(299) }));
	}

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsExactReturnsTrueWhenExactKeyIsContained) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act + Assert:
		EXPECT_TRUE(accessor.containsExact({ { { 0x44 } }, FP(100), FP(149) }));
		EXPECT_TRUE(accessor.containsExact({ { { 0x98 } }, FP(150), FP(180) }));
		EXPECT_TRUE(accessor.containsExact({ { { 0x32 } }, FP(200), FP(299) }));
	}

	PUBLIC_KEYS_ACCESSOR_TEST(ContainsExactReturnsFalseWhenExactKeyIsNotContained) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act + Assert:
		EXPECT_FALSE(accessor.containsExact({ { { 0x66 } }, FP(150), FP(180) }));
		EXPECT_FALSE(accessor.containsExact({ { { 0x98 } }, FP(140), FP(180) }));
		EXPECT_FALSE(accessor.containsExact({ { { 0x98 } }, FP(150), FP(190) }));

		EXPECT_FALSE(accessor.containsExact({ { { 0x66 } }, FP(140), FP(190) }));
	}

	// endregion

	// region PublicKeysAccessor - get / getAll

	PUBLIC_KEYS_ACCESSOR_TEST(GetReturnsKeyAtIndex) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		auto pinnedPublicKey0 = accessor.get(0);
		auto pinnedPublicKey1 = accessor.get(1);

		// Assert:
		EXPECT_EQ(model::PinnedVotingKey({ { { 0x44 } }, FP(100), FP(149) }), pinnedPublicKey0);
		EXPECT_EQ(model::PinnedVotingKey({ { { 0x32 } }, FP(200), FP(299) }), pinnedPublicKey1);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(GetAllReturnsNoKeysWhenEmpty) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act:
		auto pinnedPublicKeys = accessor.getAll();

		// Assert:
		EXPECT_TRUE(pinnedPublicKeys.empty());
	}

	PUBLIC_KEYS_ACCESSOR_TEST(GetAllReturnsAllKeysWhenNotEmpty) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		auto pinnedPublicKeys = accessor.getAll();

		// Assert:
		std::vector<model::PinnedVotingKey> expectedPinnedPublicKeys{
			{ { { 0x44 } }, FP(100), FP(149) },
			{ { { 0x32 } }, FP(200), FP(299) }
		};
		EXPECT_EQ(expectedPinnedPublicKeys, pinnedPublicKeys);
	}

	// endregion

	// region PublicKeysAccessor - add / remove

	PUBLIC_KEYS_ACCESSOR_TEST(CanAddMultipleKeys) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act:
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Assert:
		ASSERT_EQ(3u, accessor.size());
		EXPECT_EQ(VotingKey{ { 0x44 } }, accessor.get(0).VotingKey);
		EXPECT_EQ(VotingKey{ { 0x98 } }, accessor.get(1).VotingKey);
		EXPECT_EQ(VotingKey{ { 0x32 } }, accessor.get(2).VotingKey);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CannotAddOutOfOrderKeys) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });

		// Act + Assert:
		EXPECT_THROW(accessor.add({ { { 0x44 } }, FP(100), FP(110) }), catapult_invalid_argument);
		EXPECT_THROW(accessor.add({ { { 0x44 } }, FP(100), FP(149) }), catapult_invalid_argument);
	}

	PUBLIC_KEY_ACCESSOR_TEST(CannotAddKeyWithOverlappingRange) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });

		// Act + Assert:
		EXPECT_THROW(accessor.add({ { { 0x11 } }, FP(100), FP(150) }), catapult_invalid_argument);
		EXPECT_THROW(accessor.add({ { { 0x11 } }, FP(100), FP(165) }), catapult_invalid_argument);

		EXPECT_THROW(accessor.add({ { { 0x22 } }, FP(165), FP(200) }), catapult_invalid_argument);
		EXPECT_THROW(accessor.add({ { { 0x22 } }, FP(180), FP(200) }), catapult_invalid_argument);

		EXPECT_THROW(accessor.add({ { { 0x33 } }, FP(100), FP(200) }), catapult_invalid_argument);
		EXPECT_THROW(accessor.add({ { { 0x33 } }, FP(160), FP(170) }), catapult_invalid_argument);
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CannotRemoveWhenNoKeysAreSet) {
		// Arrange:
		PublicKeysAccessor accessor;

		// Act:
		auto result = accessor.remove({ { { 0x44 } }, FP(100), FP(149) });

		// Assert:
		EXPECT_FALSE(result);

		EXPECT_EQ(0u, accessor.size());
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanRemoveWhenExactMatchIsFound) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		auto result = accessor.remove({ { { 0x98 } }, FP(150), FP(180) });

		// Assert:
		EXPECT_TRUE(result);

		ASSERT_EQ(2u, accessor.size());
		EXPECT_EQ(VotingKey{ { 0x44 } }, accessor.get(0).VotingKey);
		EXPECT_EQ(VotingKey{ { 0x32 } }, accessor.get(1).VotingKey);
	}

	namespace {
		void AssertCannotRemove(const model::PinnedVotingKey& key) {
			// Arrange:
			PublicKeysAccessor accessor;
			accessor.add({ { { 0x44 } }, FP(100), FP(149) });
			accessor.add({ { { 0x98 } }, FP(150), FP(180) });
			accessor.add({ { { 0x32 } }, FP(200), FP(299) });

			// Act:
			auto result = accessor.remove(key);

			// Assert:
			EXPECT_FALSE(result);

			ASSERT_EQ(3u, accessor.size());
			EXPECT_EQ(VotingKey{ { 0x44 } }, accessor.get(0).VotingKey);
			EXPECT_EQ(VotingKey{ { 0x98 } }, accessor.get(1).VotingKey);
			EXPECT_EQ(VotingKey{ { 0x32 } }, accessor.get(2).VotingKey);
		}
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CannotRemoveWhenPartialMatchIsFound) {
		AssertCannotRemove({ { { 0x88 } }, FP(150), FP(180) });
		AssertCannotRemove({ { { 0x98 } }, FP(151), FP(180) });
		AssertCannotRemove({ { { 0x98 } }, FP(150), FP(170) });
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CannotRemoveWhenNoMatchIsFound) {
		AssertCannotRemove({ { { 0x65 } }, FP(191), FP(192) });
	}

	PUBLIC_KEYS_ACCESSOR_TEST(CanRemoveAllKeys) {
		// Arrange:
		PublicKeysAccessor accessor;
		accessor.add({ { { 0x44 } }, FP(100), FP(149) });
		accessor.add({ { { 0x98 } }, FP(150), FP(180) });
		accessor.add({ { { 0x32 } }, FP(200), FP(299) });

		// Act:
		auto result1 = accessor.remove({ { { 0x98 } }, FP(150), FP(180) });
		auto result2 = accessor.remove({ { { 0x44 } }, FP(100), FP(149) });
		auto result3 = accessor.remove({ { { 0x32 } }, FP(200), FP(299) });

		// Assert:
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_TRUE(result3);

		EXPECT_EQ(0u, accessor.size());
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
		EXPECT_EQ(0u, keys.voting().size());

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
		EXPECT_EQ(0u, keys.voting().size());

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
		EXPECT_EQ(0u, keys.voting().size());

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
		EXPECT_EQ(0u, keys.voting().size());

		EXPECT_EQ(key, keys.vrf().get());
	}

	TEST(TEST_CLASS, CanSetVotingPublicKey) {
		// Arrange:
		auto key = test::GenerateRandomPackedStruct<model::PinnedVotingKey>();
		AccountPublicKeys keys;

		// Act:
		keys.voting().add(key);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::Unset, keys.mask());

		// - one key is set
		EXPECT_FALSE(!!keys.linked());
		EXPECT_FALSE(!!keys.node());
		EXPECT_FALSE(!!keys.vrf());

		ASSERT_EQ(1u, keys.voting().size());
		EXPECT_EQ(key, keys.voting().get(0));
	}

	TEST(TEST_CLASS, CanSetAllKeys) {
		// Arrange:
		auto linkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto nodePublicKey = test::GenerateRandomByteArray<Key>();
		auto vrfPublicKey = test::GenerateRandomByteArray<Key>();
		auto votingPublicKey = test::GenerateRandomPackedStruct<model::PinnedVotingKey>();
		AccountPublicKeys keys;

		// Act:
		keys.linked().set(linkedPublicKey);
		keys.node().set(nodePublicKey);
		keys.vrf().set(vrfPublicKey);
		keys.voting().add(votingPublicKey);

		// Assert:
		EXPECT_EQ(AccountPublicKeys::KeyType::All, keys.mask());

		// - one key is set
		EXPECT_TRUE(!!keys.linked());
		EXPECT_TRUE(!!keys.node());
		EXPECT_TRUE(!!keys.vrf());

		EXPECT_EQ(linkedPublicKey, keys.linked().get());
		EXPECT_EQ(nodePublicKey, keys.node().get());
		EXPECT_EQ(vrfPublicKey, keys.vrf().get());

		ASSERT_EQ(1u, keys.voting().size());
		EXPECT_EQ(votingPublicKey, keys.voting().get(0));
	}

	// endregion
}}
