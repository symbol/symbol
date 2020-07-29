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

#include "catapult/crypto/OpensslContexts.h"
#include "catapult/crypto/AesDecrypt.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/evp.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

#define TEST_CLASS OpensslContextsTests

	// region OpensslDigestContext

	TEST(TEST_CLASS, Digest_CanDispatchSuccess) {
		// Arrange:
		OpensslDigestContext context;

		// Act + Assert:
		EXPECT_NO_THROW(context.dispatch(EVP_DigestInit_ex, EVP_sha256(), nullptr));
	}

	TEST(TEST_CLASS, Digest_CanDispatchFailure) {
		// Arrange:
		OpensslDigestContext context;

		// Act + Assert:
		EXPECT_THROW(context.dispatch(EVP_DigestInit_ex, nullptr, nullptr), catapult_runtime_error);
	}

	// endregion

	// region OpensslCipherContext

	namespace {
		class AesCbc256 {
		public:
			struct IV_tag { static constexpr size_t Size = 16; };
			using IV = utils::ByteArray<IV_tag>;
		};
	}

	TEST(TEST_CLASS, Cipher_CanDispatchSuccess) {
		// Arrange:
		OpensslCipherContext context;
		SharedKey key;
		AesCbc256::IV iv;

		// Act + Assert:
		EXPECT_NO_THROW(context.dispatch(EVP_DecryptInit_ex, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()));
	}

	TEST(TEST_CLASS, Cipher_CanDispatchFailure) {
		// Arrange:
		OpensslCipherContext context;
		SharedKey key;
		AesCbc256::IV iv;

		// Act + Assert:
		EXPECT_THROW(context.dispatch(EVP_DecryptInit_ex, nullptr, nullptr, key.data(), iv.data()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, Cipher_CanTryDispatchSuccess) {
		// Arrange:
		OpensslCipherContext context;
		SharedKey key;
		AesCbc256::IV iv;

		// Act:
		auto result = context.tryDispatch(EVP_DecryptInit_ex, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

		// Assert:
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, Cipher_CanTryDispatchFailure) {
		// Arrange:
		OpensslCipherContext context;
		SharedKey key;
		AesCbc256::IV iv;

		// Act:
		auto result = context.tryDispatch(EVP_DecryptInit_ex, nullptr, nullptr, key.data(), iv.data());

		// Assert:
		EXPECT_FALSE(result);
	}

	// endregion
}}
