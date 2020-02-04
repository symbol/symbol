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

	// the purpose of this test is to verify that result of HashPrivateKey matches sha512 hash
	TEST(TEST_CLASS, PassesShaVector) {
		// Arrange:
		auto privateKeyString = std::string("8CCB08D2A1A282AA8CC99902ECAF0F67A9F21CFFE28005CB27FCF129E963F99D");
		auto privateKey = PrivateKey::FromString(privateKeyString);

		// Act:
		Hash512 hash;
		HashPrivateKey(privateKey, hash);

		// Assert:
		Hash512 expectedHash;
		Sha512(test::HexStringToVector(privateKeyString), expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}
}}
