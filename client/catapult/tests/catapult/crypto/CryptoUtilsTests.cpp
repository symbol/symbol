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

#include "catapult/crypto/CryptoUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/PrivateKey.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS CryptoUtilsTests

	// the purpose of this test is to verify that:
	// a) in case of !SIGNATURE_SCHEME_NIS1: result of HashPrivateKey matches 512-bit sha3 hash
	// b) in case of  SIGNATURE_SCHEME_NIS1: result of HashPrivateKey matches 512-bit keccak hash
	TEST(TEST_CLASS, PassesShaVector) {
		// Arrange:
		auto privateKeyString = std::string("9F2FCC7C90DE090D6B87CD7E9718C1EA6CB21118FC2D5DE9F97E5DB6AC1E9C10");
		auto privateKey = PrivateKey::FromString(privateKeyString);

		// Act:
		Hash512 hash;
		HashPrivateKey(privateKey, hash);

		// Assert:
		Hash512 expectedHash;
#ifdef SIGNATURE_SCHEME_NIS1
		Keccak_512(test::ToVector(privateKeyString), expectedHash);
#else
		Sha3_512(test::ToVector(privateKeyString), expectedHash);
#endif

		EXPECT_EQ(expectedHash, hash);
	}
}}
