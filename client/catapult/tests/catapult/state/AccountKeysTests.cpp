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
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountKeysTests

	// region utils + traits

	namespace {
		constexpr auto Num_Key_Types = utils::to_underlying_type(AccountKeyType::Count);

		void AssertEmpty(const AccountKeys& keys) {
			// Assert:
			EXPECT_EQ(0u, keys.size());

			// - all keys are unset
			for (auto i = 0u; i < Num_Key_Types; ++i) {
				auto keyType = static_cast<AccountKeyType>(i);
				EXPECT_FALSE(keys.contains(keyType)) << i;
				EXPECT_EQ(Key(), keys.get(keyType)) << i;
			}
		}

		void AssertSingleKeyIsSet(const AccountKeys& keys, AccountKeyType keyType, const Key& key) {
			EXPECT_EQ(1u, keys.size());

			EXPECT_TRUE(keys.contains(keyType));
			EXPECT_EQ(key, keys.get(keyType));

			// - other keys are unset
			for (auto i = 0u; i < Num_Key_Types; ++i) {
				auto otherKeyType = static_cast<AccountKeyType>(i);
				if (keyType == otherKeyType)
					continue;

				EXPECT_FALSE(keys.contains(otherKeyType)) << i;
				EXPECT_EQ(Key(), keys.get(otherKeyType)) << i;
			}
		}
	}

#define KEY_TYPE_BASED_TEST(TEST_NAME) \
	template<AccountKeyType Key_Type> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Linked) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountKeyType::Linked>(); } \
	TEST(TEST_CLASS, TEST_NAME##_VRF) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountKeyType::VRF>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Voting) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountKeyType::Voting>(); } \
	template<AccountKeyType Key_Type> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region construction + assignment

	TEST(TEST_CLASS, CanCreateEmptyContainer) {
		// Act:
		AccountKeys keys;

		// Assert:
		AssertEmpty(keys);
	}

	namespace {
		AccountKeys CreateKeysForConstructionTests() {
			AccountKeys keys;
			keys.set(AccountKeyType::Linked, { { 0x44 } });
			return keys;
		}

		void AssertCopied(const AccountKeys& keys, const AccountKeys& keysCopy) {
			// Assert: the copy is detached from the original
			EXPECT_EQ(1u, keys.size());
			EXPECT_EQ(Key{ { 0x44 } }, keys.get(AccountKeyType::Linked));

			EXPECT_EQ(2u, keysCopy.size());
			EXPECT_EQ(Key{ { 0x44 } }, keysCopy.get(AccountKeyType::Linked));
			EXPECT_EQ(Key{ { 0x32 } }, keysCopy.get(AccountKeyType::Voting));
		}

		void AssertMoved(const AccountKeys& keys, const AccountKeys& keysMoved) {
			// Assert: the original values are moved into the copy
			EXPECT_EQ(0u, keys.size());

			EXPECT_EQ(1u, keysMoved.size());
			EXPECT_EQ(Key{ { 0x44 } }, keysMoved.get(AccountKeyType::Linked));
		}
	}

	TEST(TEST_CLASS, CanCopyConstruct) {
		// Arrange:
		auto keys = CreateKeysForConstructionTests();

		// Act:
		AccountKeys keysCopy(keys);
		keysCopy.set(AccountKeyType::Voting, { { 0x32 } });

		// Assert:
		AssertCopied(keys, keysCopy);
	}

	TEST(TEST_CLASS, CanMoveConstruct) {
		// Arrange:
		auto keys = CreateKeysForConstructionTests();

		// Act:
		AccountKeys keysMoved(std::move(keys));

		// Assert:
		AssertMoved(keys, keysMoved);
	}

	TEST(TEST_CLASS, CanCopyAssign) {
		// Arrange:
		auto keys = CreateKeysForConstructionTests();

		// Act:
		AccountKeys keysCopy;
		const auto& assignResult = keysCopy = keys;
		keysCopy.set(AccountKeyType::Voting, { { 0x32 } });

		// Assert:
		EXPECT_EQ(&keysCopy, &assignResult);
		AssertCopied(keys, keysCopy);
	}

	TEST(TEST_CLASS, CanMoveAssign) {
		// Arrange:
		auto keys = CreateKeysForConstructionTests();

		// Act:
		AccountKeys keysMoved;
		const auto& assignResult = keysMoved = std::move(keys);

		// Assert:
		EXPECT_EQ(&keysMoved, &assignResult);
		AssertMoved(keys, keysMoved);
	}

	// endregion

	// region set / unset

	KEY_TYPE_BASED_TEST(CanSetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		// Act:
		keys.set(Key_Type, key);

		// Assert:
		AssertSingleKeyIsSet(keys, Key_Type, key);
	}

	KEY_TYPE_BASED_TEST(CannotResetKeyWithValue) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		keys.set(Key_Type, key);

		// Act + Assert:
		EXPECT_THROW(keys.set(Key_Type, key), catapult_invalid_argument);
	}

	KEY_TYPE_BASED_TEST(CanUnsetKey) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		keys.set(Key_Type, key);

		// Act:
		keys.unset(Key_Type);

		// Assert:
		AssertEmpty(keys);
	}

	KEY_TYPE_BASED_TEST(CanUnsetAlreadyUnsetKey) {
		// Arrange:
		AccountKeys keys;

		// Act:
		keys.unset(Key_Type);

		// Assert:
		AssertEmpty(keys);
	}

	KEY_TYPE_BASED_TEST(CanSetKeyAfterUnset) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountKeys keys;

		keys.set(Key_Type, test::GenerateRandomByteArray<Key>());
		keys.unset(Key_Type);

		// Act:
		keys.set(Key_Type, key);

		// Assert:
		AssertSingleKeyIsSet(keys, Key_Type, key);
	}

	// endregion

	// region set multiple

	TEST(TEST_CLASS, CanSetAllKeys) {
		// Arrange:
		auto keyVector = test::GenerateRandomDataVector<Key>(Num_Key_Types);
		AccountKeys keys;

		// Act:
		for (auto i = 0u; i < Num_Key_Types; ++i)
			keys.set(static_cast<AccountKeyType>(i), keyVector[i]);

		// Assert:
		EXPECT_EQ(Num_Key_Types, keys.size());

		for (auto i = 0u; i < Num_Key_Types; ++i) {
			auto keyType = static_cast<AccountKeyType>(i);
			EXPECT_TRUE(keys.contains(keyType)) << i;
			EXPECT_EQ(keyVector[i], keys.get(keyType)) << i;
		}
	}

	// endregion
}}
