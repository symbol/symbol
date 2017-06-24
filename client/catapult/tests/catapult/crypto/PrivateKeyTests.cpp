#include "catapult/crypto/PrivateKey.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

	namespace {
		constexpr Key Zero_Key = Key();

		template<typename T>
		constexpr bool IsZeroKey(T memory) {
			return std::equal(Zero_Key.cbegin(), Zero_Key.cend(), memory);
		}

		Key GenerateRandomRawKey() {
			return test::GenerateRandomData<Key_Size>();
		}

		PrivateKey CreatePrivateKey(const Key& rawKey) {
			auto i = 0u;
			return PrivateKey::Generate([&i, &rawKey]() { return rawKey[i++]; });
		}

		PrivateKey CreateRandomPrivateKey() {
			return PrivateKey::Generate(test::RandomByte);
		}
	}

	TEST(PrivateKeyTests, CanCreateDefaultInitializedPrivateKey) {
		// Act:
		auto key = PrivateKey();

		// Assert:
		EXPECT_EQ(Key_Size, key.size());
		// nothing else to validate since std::array is default initialized with garbage
	}

	TEST(PrivateKeyTests, CanCreatePrivateKeyWithRawKey) {
		// Arrange:
		auto rawKey = GenerateRandomRawKey();

		// Act:
		auto key = CreatePrivateKey(rawKey);

		// Assert:
		EXPECT_EQ(Key_Size, key.size());
		EXPECT_TRUE(std::equal(rawKey.cbegin(), rawKey.cend(), key.data(), key.data() + key.size())); // raw data
		EXPECT_TRUE(std::equal(rawKey.cbegin(), rawKey.cend(), key.cbegin(), key.cend())); // const iterator
	}

	TEST(PrivateKeyTests, DestructorZerosOutBackingMemory) {
		// Arrange: call placement new and move constructor
		auto keyData = GenerateRandomRawKey();
		uint8_t keyMemory[sizeof(PrivateKey)];
		auto pKey = new (keyMemory) PrivateKey;
		*pKey = CreatePrivateKey(keyData);

		// Sanity: the key's backing memory is non zero
		EXPECT_FALSE(IsZeroKey(keyMemory));

		// Act: destroy the key
		pKey->~PrivateKey();

		// Assert: the key's backing memory is zero
		EXPECT_TRUE(IsZeroKey(keyMemory));
	}

	TEST(PrivateKeyTests, MoveConstructorZerosOutBackingMemoryOfSource) {
		// Arrange: call placement new and move constructor
		auto keyData = GenerateRandomRawKey();
		uint8_t keyMemory[sizeof(PrivateKey)];
		auto pKey = new (keyMemory) PrivateKey;
		*pKey = CreatePrivateKey(keyData);

		// Sanity: the key's backing memory is non zero
		EXPECT_FALSE(IsZeroKey(keyMemory));

		// Act: trigger the move constructor
		auto movedKey = std::move(*pKey);

		// Assert:
		// - the source's backing memory is zero
		EXPECT_TRUE(IsZeroKey(keyMemory));
		// - the destination's backing memory is equal to the moved key
		EXPECT_TRUE(std::equal(keyData.cbegin(), keyData.cend(), movedKey.data(), movedKey.data() + movedKey.size()));
	}

	TEST(PrivateKeyTests, MoveAssignmentZerosOutBackingMemoryOfSource) {
		// Arrange: call placement new and move constructor
		auto keyData = GenerateRandomRawKey();
		uint8_t keyMemory[sizeof(PrivateKey)];
		auto pKey = new (keyMemory) PrivateKey;
		*pKey = CreatePrivateKey(keyData);

		// Sanity: the key's backing memory is non zero
		EXPECT_FALSE(IsZeroKey(keyMemory));

		// Act: trigger the move assignment operator
		PrivateKey movedKey;
		const auto& assignResult = (movedKey = std::move(*pKey));

		// Assert:
		// - the source's backing memory is zero
		EXPECT_TRUE(IsZeroKey(keyMemory));
		// - the destination's backing memory is equal to the moved key
		EXPECT_TRUE(std::equal(keyData.cbegin(), keyData.cend(), movedKey.data(), movedKey.data() + movedKey.size()));
		// - the assignment operator returned the correct reference
		EXPECT_EQ(&movedKey, &assignResult);
	}

	namespace {
		void AssertCannotCreatePrivateKeyFromStringWithSize(size_t size, char keyFirstChar) {
			// Arrange:
			auto rawKeyString = test::GenerateRandomHexString(size);
			rawKeyString[0] = keyFirstChar;

			// Act / Assert: key creation should fail but string should not be cleared
			EXPECT_THROW(
				PrivateKey::FromString(rawKeyString),
				catapult_invalid_argument) << "string size: " << size;
			EXPECT_EQ(keyFirstChar, rawKeyString[0]);
		}

		void AssertCannotCreatePrivateKeyFromStringWithInvalidCharacter(char invalidChar) {
			// Arrange:
			auto rawKeyString = test::GenerateRandomHexString(Key_Size * 2);
			rawKeyString[Key_Size] = invalidChar;

			// Act / Assert: key creation should fail but string should not be cleared
			EXPECT_THROW(
				PrivateKey::FromString(rawKeyString),
				catapult_invalid_argument) << "invalid char: " << invalidChar;
			EXPECT_EQ(invalidChar, rawKeyString[Key_Size]);
		}

		void AssertCannotCreatePrivateKeyFromSecureStringWithSize(size_t size) {
			// Arrange:
			std::string zeroString(size, '\0');
			auto rawKeyString = test::GenerateRandomHexString(size);

			// Act / Assert: key creation should fail but string should still be cleared
			EXPECT_THROW(
				PrivateKey::FromStringSecure(&rawKeyString[0], rawKeyString.size()),
				catapult_invalid_argument) << "string size: " << size;
			EXPECT_EQ(zeroString, rawKeyString);
		}

		void AssertCannotCreatePrivateKeyFromSecureStringWithInvalidCharacter(char invalidChar) {
			// Arrange:
			std::string zeroString(Key_Size * 2, '\0');
			auto rawKeyString = test::GenerateRandomHexString(Key_Size * 2);
			rawKeyString[Key_Size] = invalidChar;

			// Act / Assert: key creation should fail but string should still be cleared
			EXPECT_THROW(
				PrivateKey::FromStringSecure(&rawKeyString[0], rawKeyString.size()),
				catapult_invalid_argument) << "invalid char: " << invalidChar;
			EXPECT_EQ(zeroString, rawKeyString);
		}
	}

	TEST(PrivateKeyTests, CannotCreatePrivateKeyFromIncorrectlySizedString) {
		// Assert:
		AssertCannotCreatePrivateKeyFromStringWithSize(0, '\0');
		AssertCannotCreatePrivateKeyFromStringWithSize(50, 'c');
		AssertCannotCreatePrivateKeyFromStringWithSize(120, 'd');
	}

	TEST(PrivateKeyTests, CannotCreatePrivateKeyFromIncorrectlySizedSecureString) {
		// Assert:
		AssertCannotCreatePrivateKeyFromSecureStringWithSize(0);
		AssertCannotCreatePrivateKeyFromSecureStringWithSize(50);
		AssertCannotCreatePrivateKeyFromSecureStringWithSize(120);
	}

	TEST(PrivateKeyTests, CannotCreatePrivateKeyFromStringWithInvalidCharacter) {
		// Assert:
		AssertCannotCreatePrivateKeyFromStringWithInvalidCharacter('g');
		AssertCannotCreatePrivateKeyFromStringWithInvalidCharacter('-');
	}

	TEST(PrivateKeyTests, CannotCreatePrivateKeyFromSecureStringWithInvalidCharacter) {
		// Assert:
		AssertCannotCreatePrivateKeyFromSecureStringWithInvalidCharacter('g');
		AssertCannotCreatePrivateKeyFromSecureStringWithInvalidCharacter('-');
	}

	TEST(PrivateKeyTests, CanCreatePrivateKeyFromString) {
		// Arrange:
		auto rawKeyString = std::string("3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb");

		// Act:
		auto key = PrivateKey::FromString(rawKeyString);

		// Assert:
		Key rawKey{ {
			0x34, 0x85, 0xd9, 0x8e, 0xfd, 0x7e, 0xb0, 0x7a, 0xda, 0xfc, 0xfd, 0x1a, 0x15, 0x7d, 0x89, 0xde,
			0x27, 0x96, 0xa9, 0x5e, 0x78, 0x08, 0x13, 0xc0, 0x25, 0x8a, 0xf3, 0xf5, 0xf8, 0x4e, 0xd8, 0xcb
		} };
		EXPECT_TRUE(std::equal(rawKey.cbegin(), rawKey.cend(), key.data(), key.data() + key.size()));

		// - note that the factory function should not have zeroed out the input string
		EXPECT_EQ('3', rawKeyString[0]);
	}

	TEST(PrivateKeyTests, CanCreatePrivateKeyFromStringSecure) {
		// Arrange:
		std::string zeroString(Key_Size * 2, '\0');
		auto rawKeyString = std::string("3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb");

		// Act:
		auto key = PrivateKey::FromStringSecure(&rawKeyString[0], rawKeyString.size());

		// Assert:
		Key rawKey{ {
			0x34, 0x85, 0xd9, 0x8e, 0xfd, 0x7e, 0xb0, 0x7a, 0xda, 0xfc, 0xfd, 0x1a, 0x15, 0x7d, 0x89, 0xde,
			0x27, 0x96, 0xa9, 0x5e, 0x78, 0x08, 0x13, 0xc0, 0x25, 0x8a, 0xf3, 0xf5, 0xf8, 0x4e, 0xd8, 0xcb
		} };
		EXPECT_TRUE(std::equal(rawKey.cbegin(), rawKey.cend(), key.data(), key.data() + key.size()));

		// - note that the factory function should have zeroed out the input string
		EXPECT_EQ(zeroString, rawKeyString);
	}

	TEST(PrivateKeyTests, CanCreatePrivateKeyFromGenerator) {
		// Act:
		auto seed = 0;
		auto key = PrivateKey::Generate([&seed]() {
			auto val = seed / 8 + seed % 8;
			++seed;
			return val;
		});

		// Assert:
		Key rawKey{ {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a
		} };
		EXPECT_TRUE(std::equal(rawKey.cbegin(), rawKey.cend(), key.data(), key.data() + key.size()));
	}

	namespace {
		const char* Default_Key = "default";

		std::unordered_map<std::string, PrivateKey> GenerateEqualityInstanceMap() {
			auto rawKey = GenerateRandomRawKey();
			std::unordered_map<std::string, PrivateKey> map;
			map.emplace(Default_Key, CreatePrivateKey(rawKey));
			map.emplace("same-raw", CreatePrivateKey(rawKey));
			map.emplace("diff-raw", CreateRandomPrivateKey());
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "same-raw" };
		}
	}

	TEST(PrivateKeyTests, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(
			Default_Key,
			GenerateEqualityInstanceMap(),
			GetEqualTags());
	}

	TEST(PrivateKeyTests, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(
			Default_Key,
			GenerateEqualityInstanceMap(),
			GetEqualTags());
	}
}}
