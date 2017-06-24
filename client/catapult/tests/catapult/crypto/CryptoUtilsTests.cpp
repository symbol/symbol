#include "catapult/crypto/CryptoUtils.h"
#include "catapult/crypto/PrivateKey.h"
#include "tests/TestHarness.h"
#include <ref10/crypto_hash_sha512.h>

namespace catapult { namespace crypto {

	// the purpose of this test is to verify that:
	// a) in case of !NIS1_COMPATIBLE_SIGNATURES:
	//    result of HashPrivateKey, matches 512-bit sha3 hash (tested in sha3_512 tests)
	// b) in case of NIS1_COMPATIBLE_SIGNATURES
	//    result of HashPrivateKey, matches 512-bit sha3 hash of REVERSED data
	TEST(HashPrivateKeyTests, PassesShaVector) {
		// Arrange:
		auto privateKey = PrivateKey::FromString("9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10");

		// Act:
		Hash512 hash;
		HashPrivateKey(privateKey, hash);

		// Assert:
#ifdef NIS1_COMPATIBLE_SIGNATURES
		// pass reversed key
		auto shaVector = test::ToVector("109C1EACB65D7EF9E95D2DFC1811B26CEAC118977ECD876B0D09DE907CCC2F9F");
#else
		auto shaVector = test::ToVector("9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10");
#endif
		Hash512 expectedHash;
		crypto_hash_sha512(expectedHash.data(), shaVector.data(), shaVector.size());

		ASSERT_EQ(test::ToHexString(expectedHash), test::ToHexString(hash));
	}
}}
