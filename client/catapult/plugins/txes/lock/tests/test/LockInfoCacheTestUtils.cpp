#include "LockInfoCacheTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	void AssertEqualLockInfo(const model::LockInfo& lhs, const model::LockInfo& rhs) {
		EXPECT_EQ(lhs.Account, rhs.Account);
		EXPECT_EQ(lhs.MosaicId, rhs.MosaicId);
		EXPECT_EQ(lhs.Amount, rhs.Amount);
		EXPECT_EQ(lhs.Height, rhs.Height);
		EXPECT_EQ(lhs.Status, rhs.Status);
	}

	BasicHashLockInfoTestTraits::ValueType BasicHashLockInfoTestTraits::CreateLockInfo(Height height) {
		return model::HashLockInfo(
				GenerateRandomData<Key_Size>(),
				MosaicId(Random()),
				Amount(Random()),
				height,
				GenerateRandomData<Hash256_Size>());
	}

	BasicHashLockInfoTestTraits::ValueType BasicHashLockInfoTestTraits::CreateLockInfo() {
		return CreateLockInfo(test::GenerateRandomValue<Height>());
	}

	void BasicHashLockInfoTestTraits::AssertEqual(const ValueType& lhs, const ValueType& rhs) {
		AssertEqualLockInfo(lhs, rhs);
		EXPECT_EQ(lhs.Hash, rhs.Hash);
	}

	BasicSecretLockInfoTestTraits::ValueType BasicSecretLockInfoTestTraits::CreateLockInfo(Height height) {
		return model::SecretLockInfo(
				GenerateRandomData<Key_Size>(),
				MosaicId(Random()),
				Amount(Random()),
				height,
				static_cast<model::LockHashAlgorithm>(RandomByte()),
				GenerateRandomData<Hash512_Size>(),
				GenerateRandomData<Address_Decoded_Size>());
	}

	BasicSecretLockInfoTestTraits::ValueType BasicSecretLockInfoTestTraits::CreateLockInfo() {
		return CreateLockInfo(test::GenerateRandomValue<Height>());
	}

	void BasicSecretLockInfoTestTraits::AssertEqual(const ValueType& lhs, const ValueType& rhs) {
		AssertEqualLockInfo(lhs, rhs);
		EXPECT_EQ(lhs.HashAlgorithm, rhs.HashAlgorithm);
		EXPECT_EQ(lhs.Secret, rhs.Secret);
		EXPECT_EQ(lhs.Recipient, rhs.Recipient);
	}
}}
